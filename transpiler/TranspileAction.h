#pragma once

#include <clang/Frontend/FrontendActions.h>
#include <clang/ast/ASTConsumer.h>

#include "llvm/Support/Program.h"
#include "llvm/Support/InitLLVM.h"
#include <regex>

#include <clang/astmatchers/ASTMatchers.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include "clang/Frontend/TextDiagnosticPrinter.h"

#include "KernelValidator.h"
#include "callbacks/BindingPointCallback.h"
#include "callbacks/CpuMethodCallback.h"
#include "matchers/LocalSizeCallback.h"
#include "matchers/KernelLocatorCallback.h"


#include <vector>


#include "PendingEdit.h"
#include "KernelStruct.h"
#include "KernelRewriter.h"

class TranspileAction : public clang::ASTFrontendAction {
public:
	TranspileAction(const std::string_view& outputDir)
		: m_OutputDir{ outputDir }
	{

	}

	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& CI, clang::StringRef) override {
		llvm::errs() << "Creating AST customer.\n";
		RewriterObj.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
		m_pASTContext = &CI.getASTContext();
		Validator = std::make_unique<KernelValidator>(CI.getASTContext());

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
			hasDescendant(
				fieldDecl(
					hasName("local_size")
				).bind("localSizeVar")
			),
			hasMethod(
				cxxMethodDecl(hasName("main"))
			)
		).bind("kernelStruct");


		Finder.addMatcher(kernelStructMatcher, &KernelLocator);
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

	void validateKernel(KernelStruct& kernel)
	{
		const CXXRecordDecl* kernelStruct = kernel.getKernelRecordDecl();
		Validator->TraverseDecl(const_cast<CXXRecordDecl*>(kernelStruct));
	}

	void EndSourceFileAction() override {
		auto& foundKernels = KernelLocator.getKernels();
		if (foundKernels.size() == 0) {
			llvm::errs() << "No kernel struct annotated; skipping.\n";
			return;
		}
		llvm::errs() << "Found #kernels:" << foundKernels.size() << "\n";

		// validate all the kernels.
		for (KernelStruct& ks : foundKernels)
		{
			validateKernel(ks);
		}

		if (!Validator->isValid())
		{
			llvm::errs() << "Validation failed inside kernel; aborting transpilation.\n";
			return;
		}
		else {
			llvm::errs() << "All kernels are valid.\n";
		}

		SourceManager& sourceManager = RewriterObj.getSourceMgr();
		const LangOptions& languageOptions = RewriterObj.getLangOpts();

		std::vector<PendingEdit>& pendingEdits = KernelLocator.getPendingEdits();
		auto rewriter = std::make_unique<KernelRewriter>(m_pASTContext, pendingEdits);
		for (KernelStruct& ks : foundKernels)
		{
			const CXXRecordDecl* kernelStruct = ks.getKernelRecordDecl();
			rewriter->TraverseDecl(const_cast<CXXRecordDecl*>(kernelStruct));
			// apply all the edits.
			std::sort(pendingEdits.begin(), pendingEdits.end(), [](auto& a, auto& b) {
				return a.m_Range.getBegin() < b.m_Range.getBegin();
				});
			for (auto& e : pendingEdits)
			{
				e.replace(RewriterObj);

			}
			llvm::errs() << "Writing kernel to " << m_OutputDir << "\n";
			ks.exportComputeShader(RewriterObj, m_OutputDir);
			pendingEdits.clear();

		}
		llvm::errs() << "All kernels have been rewritten\n";
	}
private:
	std::string m_OutputDir;

	clang::Rewriter RewriterObj;
	clang::ASTContext* m_pASTContext;
	clang::ast_matchers::MatchFinder Finder;

	std::unique_ptr<KernelValidator> Validator;
	KernelLocatorCallback KernelLocator{ };
};