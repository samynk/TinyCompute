#pragma once
#include <clang/astmatchers/ASTMatchFinder.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/lex/Lexer.h>


using namespace clang;
using namespace clang::ast_matchers;
class LocalSizeCallback : public MatchFinder::MatchCallback {
public:
    LocalSizeCallback(clang::Rewriter& R) : Rewrite(R) {}

    std::optional<unsigned> extractUInt(const Expr* E, ASTContext& Ctx) {
        if (!E) return std::nullopt;
        E = E->IgnoreImplicit(); // strip casts, etc.

        // Direct integer literal
        if (const auto* IL = dyn_cast<IntegerLiteral>(E)) {
            return static_cast<unsigned>(IL->getValue().getZExtValue());
        }

        // Try constant evaluation (e.g., more complex constexpr expressions)
        Expr::EvalResult Result;
        if (E->EvaluateAsRValue(Result, Ctx)) {
            if ( Result.Val.hasValue()) {
                const llvm::APSInt AI = Result.Val.getInt();
                return static_cast<unsigned>(AI.getZExtValue());
            }
        }

        return std::nullopt;
    }

    void run(const MatchFinder::MatchResult& Result) override {
        const clang::FieldDecl* FD = Result.Nodes.getNodeAs<clang::FieldDecl>("localSizeVar");
        auto Ctx = Result.Context;
        if (!FD->hasInClassInitializer())
            return;

        const Expr* Init = FD->getInClassInitializer()->IgnoreImplicit();

        // Expect a constructor expression: sf::uvec3{18,18,1} or sf::uvec3(18,18,1)
        const auto* CCE = dyn_cast<CXXConstructExpr>(Init);
        if (!CCE)
            return;

        if (CCE->getNumArgs() != 3)
            return;

        auto xOpt = extractUInt(CCE->getArg(0), *Ctx);
        auto yOpt = extractUInt(CCE->getArg(1), *Ctx);
        auto zOpt = extractUInt(CCE->getArg(2), *Ctx);
        if (!xOpt || !yOpt || !zOpt) {
            llvm::errs() << "Failed to evaluate one of the local_size components\n";
            return;
        }

        unsigned x = *xOpt;
        unsigned y = *yOpt;
        unsigned z = *zOpt;

        llvm::outs() << "Extracted local_size: " << x << ", " << y << ", " << z << "\n";

        // Build replacement GLSL layout line
        std::string replacement =
            "layout (local_size_x = " + std::to_string(x) +
            ", local_size_y = " + std::to_string(y) +
            ", local_size_z = " + std::to_string(z) +
            ") in;";

        // Replace the entire field declaration (including initializer)
        SourceManager& SM = Ctx->getSourceManager();
        SourceLocation endLoc = Lexer::getLocForEndOfToken(
            FD->getSourceRange().getEnd(), 0, SM, Ctx->getLangOpts());
        SourceRange fullRange(FD->getSourceRange().getBegin(), endLoc);
        Rewrite.ReplaceText(fullRange, replacement);
    }

private:
    Rewriter& Rewrite;
};