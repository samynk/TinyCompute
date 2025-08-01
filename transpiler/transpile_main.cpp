




#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <llvm/Support/CommandLine.h>
#include <iostream>
#include <vector>

#include "TranspileAction.h"

using namespace clang;
using namespace clang::tooling;

static llvm::cl::OptionCategory MyToolCategory("kernel-transpiler options");

static llvm::cl::opt<std::string> OutputPath(
    "o", llvm::cl::desc("Output file"), llvm::cl::value_desc("filename"),
    llvm::cl::Required, llvm::cl::cat(MyToolCategory));

int main(int argc, const char** argv) {

    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }

    ClangTool Tool(ExpectedParser->getCompilations(),
        ExpectedParser->getSourcePathList());


    return Tool.run(newFrontendActionFactory<TranspileAction>().get());

}
