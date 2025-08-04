#pragma once

#include <clang/Frontend/FrontendActions.h>
#include <clang/ast/ASTConsumer.h>


#include <clang/astmatchers/ASTMatchers.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include "clang/Frontend/TextDiagnosticPrinter.h"

#include "KernelValidator.h"
#include "callbacks/BindingPointCallback.h"
#include "matchers/LocalSizeCallback.h"
#include "matchers/KernelLocatorCallback.h"

#include <vector>

#include "PendingEdit.h"



class TranspileAction : public clang::ASTFrontendAction {
public:
	TranspileAction() = default;

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
		).bind("kernelStruct");

		using namespace clang::ast_matchers;

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

		Finder.addMatcher(localSizeMatcher, &Callback);
		Finder.addMatcher(kernelStructMatcher, &KernelLocator);
		Finder.addMatcher(bindingPointMatcher, &bpCallback);
		Finder.addMatcher(uniformMatcher, &bpCallback);

		return Finder.newASTConsumer();
	}
	
	

	bool BeginInvocation(CompilerInstance& CI) override { 
		llvm::errs() << "Setting up diagnostics printer.\n";
		return true;
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

		// Phase 2: apply rewrites
		Finder.matchAST(getCompilerInstance().getASTContext());

		// Emit transformed source
		std::sort(pendingEdits.begin(), pendingEdits.end(), [](auto& a, auto& b) {
			return a.range.getBegin() < b.range.getBegin();
		});
		for (auto& e : pendingEdits)
		{
			RewriterObj.ReplaceText(e.range, e.replacement);
		}

		SourceRange range = kernelStruct->getSourceRange();
		SourceManager& sourceManager = RewriterObj.getSourceMgr();
		const LangOptions& languageOptions = RewriterObj.getLangOpts();

		std::string computeShader = RewriterObj.getRewrittenText(range);
		llvm::outs() << computeShader;
		
	}

private:
	clang::Rewriter RewriterObj;
	clang::ast_matchers::MatchFinder Finder;

	std::unique_ptr<KernelValidator> Validator;
	std::vector<PendingEdit> pendingEdits;

	LocalSizeCallback Callback{ pendingEdits };
	BindingPointCallback bpCallback{ pendingEdits };
	const CXXRecordDecl* FoundKernel = nullptr;
	KernelLocatorCallback KernelLocator{ FoundKernel };

	
};