#pragma once

#include <clang/Frontend/FrontendActions.h>
#include <clang/ast/ASTConsumer.h>


#include <clang/astmatchers/ASTMatchers.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include "clang/Frontend/TextDiagnosticPrinter.h"

#include "KernelValidator.h"
#include "callbacks/BindingPointCallback.h"
#include "callbacks/CpuMethodCallback.h"
#include "matchers/LocalSizeCallback.h"
#include "matchers/KernelLocatorCallback.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>


#include "PendingEdit.h"



class TranspileAction : public clang::ASTFrontendAction {
public:
	TranspileAction(const std::string_view& outputDir)
		: m_OutputDir{ outputDir }
	{

	}

	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& CI, clang::StringRef) override {
		llvm::errs() << "Creating AST customer.\n";
		RewriterObj.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
		// Phase 1: validate kernel – you would scope this to the annotated struct / function body.
		Validator = std::make_unique<KernelValidator>(CI.getASTContext());

		// Phase 2: setup matchers
		// Match a var named local_size of type vec3/uvec3 with three-element init list.
		auto localSizeMatcher = fieldDecl(
			hasName("local_size")
			//,hasInitializer(initListExpr(hasSize(3)))
			//,hasType(qualType(hasDeclaration(recordDecl(matchesName("^[ui]?vec3$")))))
		).bind("localSizeVar");

		auto kernelStructMatcher = cxxRecordDecl(
			isDefinition(),
			hasAttr(attr::Annotate)
			,
			hasDescendant(
				varDecl(
					hasName("fileLocation"),
					isConstexpr()
				).bind("fileLocation")
			),
			hasMethod(
				cxxMethodDecl(hasName("main"))
			)
		).bind("kernelStruct");

		auto bindingPointMatcher = fieldDecl(
			hasType(qualType(hasDeclaration(
				classTemplateSpecializationDecl(hasName("BindingPoint"))
			)))
		).bind("bindingField");

		auto uniformMatcher = fieldDecl(
			hasType(qualType(hasDeclaration(
				classTemplateSpecializationDecl(hasName("Uniform"))
			)))
		).bind("uniformField");

		/*auto cpuMethodMatcher = cxxMethodDecl(
			matchesName("::_.*")
		).bind("cpuMethod");*/

		Finder.addMatcher(localSizeMatcher, &Callback);
		Finder.addMatcher(kernelStructMatcher, &KernelLocator);
		Finder.addMatcher(bindingPointMatcher, &bpCallback);
		Finder.addMatcher(uniformMatcher, &bpCallback);
		//Finder.addMatcher(cpuMethodMatcher, &cpuMethodCallback);

		return Finder.newASTConsumer();
	}



	bool BeginInvocation(CompilerInstance& CI) override {
		llvm::errs() << "Setting up diagnostics printer.\n";
		return true;
	}

	std::optional<const VarDecl*> getFileLocationDeclaration(CXXRecordDecl* kernelStruct)
	{
		for (const auto* D : kernelStruct->decls()) {
			if (const auto* VD = llvm::dyn_cast<VarDecl>(D)) {
				if (VD->getName() == "fileLocation") {
					return VD;
				}
			}
		}
		return {};
	}

	std::optional<std::filesystem::path> getFileLocation(const VarDecl* fileLocVar)
	{
		if (const Expr* Init = fileLocVar->getInit()) {
			if (const auto* SL = llvm::dyn_cast<StringLiteral>(Init->IgnoreImplicit())) {
				return SL->getString().str();
			}

		}
		return {};
	}

	void EndSourceFileAction() override {
		if (!FoundKernel) {
			llvm::errs() << "No kernel struct annotated; skipping.\n";
			return;
		}

		// Now validate *only inside* the kernel definition.
		// If your validator is a RecursiveASTVisitor, traverse the record itself:

		CXXRecordDecl* kernelStruct = const_cast<CXXRecordDecl*>(FoundKernel);
		Validator->TraverseDecl(kernelStruct);


		if (!Validator->isValid()) {
			llvm::errs() << "Validation failed inside kernel; aborting transpilation.\n";
			return;
		}

		if (auto fileLocVar = getFileLocationDeclaration(kernelStruct))
		{
			if (auto fileLoc = getFileLocation(fileLocVar.value()))
			{
				// Phase 2: apply rewrites
				Finder.matchAST(getCompilerInstance().getASTContext());
				PendingEdit removeFileLoc{ fileLocVar.value()->getSourceRange(),"" }; // replace with nothing.
				pendingEdits.emplace_back(removeFileLoc);
				// Emit transformed source
				std::sort(pendingEdits.begin(), pendingEdits.end(), [](auto& a, auto& b) {
					return a.range.getBegin() < b.range.getBegin();
					});
				for (auto& e : pendingEdits)
				{
					RewriterObj.ReplaceText(e.range, e.replacement);
				}


				SourceManager& sourceManager = RewriterObj.getSourceMgr();
				const LangOptions& languageOptions = RewriterObj.getLangOpts();

				SourceRange range = kernelStruct->getBraceRange();
				SourceLocation contentStart = clang::Lexer::getLocForEndOfToken(
					sourceManager.getExpansionLoc(range.getBegin()), 0, sourceManager, languageOptions);

				SourceLocation contentEnd = sourceManager.getExpansionLoc(range.getEnd().getLocWithOffset(-1));

				std::string computeShader = RewriterObj.getRewrittenText(SourceRange{ contentStart,contentEnd });
				llvm::outs() << computeShader;


				std::filesystem::path path = m_OutputDir;
				std::filesystem::path glslFile = fileLoc.value().replace_extension("glsl");
				path /= glslFile;
				std::filesystem::create_directories(path.parent_path());

				std::ofstream ofs(path);
				ofs << "#version 430\n";
				ofs << computeShader;
				ofs.close();
			}
		}
	}

private:
	std::string m_OutputDir;

	clang::Rewriter RewriterObj;
	clang::ast_matchers::MatchFinder Finder;

	std::unique_ptr<KernelValidator> Validator;
	std::vector<PendingEdit> pendingEdits;

	LocalSizeCallback Callback{ pendingEdits };
	BindingPointCallback bpCallback{ pendingEdits };
	CpuMethodCallback cpuMethodCallback{ pendingEdits };
	const CXXRecordDecl* FoundKernel = nullptr;
	KernelLocatorCallback KernelLocator{ FoundKernel,pendingEdits };
};