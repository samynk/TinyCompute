#pragma once

#include <clang/Tooling/Tooling.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/DiagnosticIDs.h>

static llvm::cl::OptionCategory ToolCategory("transpile options");

using CustomDiagnostic = clang::DiagnosticIDs::CustomDiagDesc;

/// Phase 1: validator that rejects forbidden constructs inside a kernel.
class KernelValidator : public clang::RecursiveASTVisitor<KernelValidator> {
public:
	explicit KernelValidator(clang::ASTContext& Ctx) : Context(Ctx),
		invalidField(clang::diag::Severity::Error, "kernel field '%0' must not be a pointer or reference"),
		dynamicMemory(clang::diag::Severity::Error, "new' is not allowed in kernel, only value types are allowed (see kernel constraints)."),
		classTemplate(clang::diag::Severity::Error, "Error: class templates not allowed."),
		lambda(clang::diag::Severity::Error, "Error: lambda not allowed in kernel.")
	{}

	bool VisitCXXNewExpr(clang::CXXNewExpr* E){
		llvm::errs() << E->getBeginLoc().printToString(Context.getSourceManager()) << " Error: 'new' is not allowed in kernel, only value types are allowed.\n";
		//reportError(E->getBeginLoc(), " Error VAL01: 'new' is not allowed in kernel, only value types are allowed (see kernel constraints).");
		reportError(E->getBeginLoc(), dynamicMemory);
		Valid = false;
		return true;
	}

	bool VisitFieldDecl(clang::FieldDecl* FD){
		clang::QualType QT = FD->getType();
		if (QT->isPointerType() || QT->isReferenceType()) {
			reportError(FD->getLocation(),invalidField, FD->getName());
		}
		return true;
	}

	bool VisitLambdaExpr(clang::LambdaExpr* L) {
		llvm::errs() << "Error: lambda not allowed in kernel at "
			<< L->getBeginLoc().printToString(Context.getSourceManager()) << "\n";
		Valid = false;
		return true;
	}

	bool VisitClassTemplateDecl(clang::ClassTemplateDecl* T) {
		llvm::errs() << " Error: templates not allowed in kernel at "
			<< T->getLocation().printToString(Context.getSourceManager()) << "\n";
		Valid = false;
		return true;
	}

	void reportError(clang::SourceLocation Loc, CustomDiagnostic& diagnostic, llvm::StringRef Arg = {})
	{
		clang::DiagnosticsEngine& DE = Context.getDiagnostics();
		clang::DiagnosticIDs& ids = *DE.getDiagnosticIDs();
		unsigned diagnosticID = ids.getCustomDiagID(diagnostic);
		llvm::errs() << diagnostic.GetDescription() << ":" << Arg << "\n";
		
		clang::DiagnosticBuilder db = DE.Report(Loc, diagnosticID);
		db.AddString( Arg) ;
	}

	bool isValid() const { return Valid; }

private:
	clang::ASTContext& Context;
	CustomDiagnostic dynamicMemory;
	CustomDiagnostic lambda;
	CustomDiagnostic classTemplate;
	CustomDiagnostic invalidField;
	bool Valid = true;
};