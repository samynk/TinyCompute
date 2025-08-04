#pragma once

#include <clang/astmatchers/ASTMatchFinder.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/lex/Lexer.h>

#include "../PendingEdit.h"
#include <vector>

class KernelLocatorCallback : public MatchFinder::MatchCallback {
public:
    const clang::CXXRecordDecl*& Out;
    KernelLocatorCallback(const clang::CXXRecordDecl*& outRef, std::vector<PendingEdit>& edits) 
        : Out(outRef), m_PendingEdits{edits} 
    {
    }

    void run(const MatchFinder::MatchResult& Result) override {
        if (const clang::CXXRecordDecl* RD = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("kernelStruct")) {
            for (const auto* A : RD->specific_attrs<clang::AnnotateAttr>()) {
                if (A->getAnnotation() == "kernel") {
                    // It's a kernel—proceed.
                    llvm::outs() << "Found kernel struct: " << RD->getNameAsString() << "\n";
                    Out = RD;
                }
            }
            const clang::VarDecl* fileLoc = Result.Nodes.getNodeAs<clang::VarDecl>("fileLocation");
            const clang::Expr* init = fileLoc->getInit();
            if (const auto* SL = llvm::dyn_cast<StringLiteral>(init->IgnoreImplicit())) {
                std::string relPath = SL->getString().str(); // e.g., "gameoflife"
                llvm::errs() << "Writing this kernel to " << relPath <<"\n";
            }

            for (const auto* method : RD->methods())
            {
                llvm::errs() << "Found method:" << method->getNameAsString() << "\n";
                if (method->getNameAsString().starts_with("_"))
                {
                    PendingEdit edit{ method->getSourceRange(), "" };
                    m_PendingEdits.emplace_back(edit);
                }
            }

        }
    }
private:
    std::vector<PendingEdit>& m_PendingEdits;
};