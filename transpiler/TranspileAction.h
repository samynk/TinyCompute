#pragma once

#include <clang/Frontend/FrontendActions.h>
#include <clang/ast/ASTConsumer.h>


#include <clang/astmatchers/ASTMatchers.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include "clang/Frontend/TextDiagnosticPrinter.h"

#include "KernelValidator.h"
#include "matchers/LocalSizeCallback.h"
#include "matchers/KernelLocatorCallback.h"

class TranspileAction : public clang::ASTFrontendAction {
public:
	TranspileAction() = default;

	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& CI, clang::StringRef) override {
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

		//auto anyVarMatcher = varDecl().bind("anyVar");
		//Finder.addMatcher(anyVarMatcher, &Callback);


		Finder.addMatcher(localSizeMatcher, &Callback);
		Finder.addMatcher(kernelStructMatcher, &KernelLocator);

		return Finder.newASTConsumer();
	}



	void EndSourceFileAction() override {
		// Run the validator traversal (you'd likely restrict to code inside the kernel)
		// Validator->TraverseDecl(getCompilerInstance().getASTContext().getTranslationUnitDecl());

		if (!FoundKernel) {
			llvm::errs() << "No kernel struct annotated; skipping.\n";
			return;
		}

		// Now validate *only inside* the kernel definition.
		// If your validator is a RecursiveASTVisitor, traverse the record itself:
		Validator->TraverseDecl(const_cast<CXXRecordDecl*>(FoundKernel));

		if (!Validator->isValid()) {
			llvm::errs() << "Validation failed inside kernel; aborting transpilation.\n";
			return;
		}

		// Phase 2: apply rewrites
		Finder.matchAST(getCompilerInstance().getASTContext());

		// Emit transformed source
		RewriterObj.getEditBuffer(RewriterObj.getSourceMgr().getMainFileID()).write(llvm::outs());
	}

private:
	clang::Rewriter RewriterObj;
	clang::ast_matchers::MatchFinder Finder;

	std::unique_ptr<KernelValidator> Validator;

	LocalSizeCallback Callback{ RewriterObj };
	const CXXRecordDecl* FoundKernel = nullptr;
	KernelLocatorCallback KernelLocator{ FoundKernel };


};