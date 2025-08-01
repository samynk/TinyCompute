#pragma once

#include <clang/astmatchers/ASTMatchFinder.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/lex/Lexer.h>

class KernelLocatorCallback : public MatchFinder::MatchCallback {
public:
    const clang::CXXRecordDecl*& Out;
    KernelLocatorCallback(const clang::CXXRecordDecl*& outRef) : Out(outRef) {}

    void run(const MatchFinder::MatchResult& Result) override {
        if (const clang::CXXRecordDecl* RD = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("kernelStruct")) {
            for (const auto* A : RD->specific_attrs<clang::AnnotateAttr>()) {
                if (A->getAnnotation() == "kernel") {
                    // It's a kernel—proceed.
                    llvm::outs() << "Found kernel struct: " << RD->getNameAsString() << "\n";
                    Out = RD;
                }
            }
        }
    }
};