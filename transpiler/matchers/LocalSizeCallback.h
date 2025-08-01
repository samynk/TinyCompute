#pragma once
#include <clang/astmatchers/ASTMatchFinder.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/lex/Lexer.h>


using namespace clang;
using namespace clang::ast_matchers;
class LocalSizeCallback : public MatchFinder::MatchCallback {
public:
    LocalSizeCallback(clang::Rewriter& R) : Rewrite(R) {}

    void run(const MatchFinder::MatchResult& Result) override {
        const clang::FieldDecl* VD = Result.Nodes.getNodeAs<clang::FieldDecl>("localSizeVar");

        llvm::outs() << "Found a local_size var. " << "\n";

        if (!VD || !VD->hasInClassInitializer()) return;

        llvm::outs() << "Local size has init. " << "\n";

        const InitListExpr* ILE = dyn_cast<InitListExpr>(VD->getInClassInitializer()->IgnoreImplicit());
        if (!ILE || ILE->getNumInits() != 3) return;

        llvm::outs() << "Local size has 3 initialization values. " << "\n";

        auto extractUInt = [&](const Expr* E) -> std::optional<unsigned> {
            E = E->IgnoreImplicit();
            if (const IntegerLiteral* IL = dyn_cast<IntegerLiteral>(E)) {
                return static_cast<unsigned>(IL->getValue().getLimitedValue());
            }
            return false;
            };



        auto x = extractUInt(ILE->getInit(0));
        auto y = extractUInt(ILE->getInit(1));
        auto z = extractUInt(ILE->getInit(2));
        if (!x || !y || !z) return;

        std::string replacement =
            "layout (local_size_x = " + std::to_string(*x) +
            ", local_size_y = " + std::to_string(*y) +
            ", local_size_z = " + std::to_string(*z) +
            ") in;";

        llvm::outs() << "Replacing local_size with : " << replacement << "\n";

        SourceManager& SM = *Result.SourceManager;
        SourceLocation endLoc = clang::Lexer::getLocForEndOfToken(
            VD->getSourceRange().getEnd(), 0, SM, Result.Context->getLangOpts());
        SourceRange fullRange(VD->getSourceRange().getBegin(), endLoc);
        Rewrite.ReplaceText(fullRange, replacement);
    }

private:
    Rewriter& Rewrite;
};