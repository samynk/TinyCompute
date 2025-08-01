#pragma once

#include <clang/Tooling/Tooling.h>
#include <clang/AST/RecursiveASTVisitor.h>

static llvm::cl::OptionCategory ToolCategory("transpile options");

/// Phase 1: validator that rejects forbidden constructs inside a kernel.
class KernelValidator : public clang::RecursiveASTVisitor<KernelValidator> {
public:
    explicit KernelValidator(clang::ASTContext& Ctx) : Context(Ctx) {}

    bool VisitCXXNewExpr(clang::CXXNewExpr* E) {
        llvm::errs() << E->getBeginLoc().printToString(Context.getSourceManager()) << " Error: 'new' is not allowed in kernel, only value types are allowed.\n";
        reportError( E->getBeginLoc(), " Error VAL01: 'new' is not allowed in kernel, only value types are allowed (see kernel constraints).");
        
        Valid = false;
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

    void reportError(clang::SourceLocation Loc, llvm::StringRef Message) {
        /*clang::DiagnosticsEngine& DE = Context.getDiagnostics();
        unsigned DiagID = DE.getCustomDiagID(clang::DiagnosticsEngine::Error, "%0");
        clang::DiagnosticBuilder DB = DE.Report(Loc, DiagID);
        DB.AddString(Message);*/

        clang::SourceManager& SM = Context.getSourceManager();
        clang::PresumedLoc PLoc = SM.getPresumedLoc(Loc);
        if (PLoc.isValid()) {
            llvm::errs() << PLoc.getFilename() << "(" << PLoc.getLine() << "," << PLoc.getColumn()
                << "): " << Message;
        }
    }

    bool isValid() const { return Valid; }

private:
    clang::ASTContext& Context;
    bool Valid = true;
};