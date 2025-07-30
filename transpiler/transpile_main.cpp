#include <iostream>
#include <vector>

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

int main(int argc, char* argv[])
{

	std::cout << "Tool called with " << (argc-1) << " files." << std::endl;

    std::vector<std::string> fileList;
    std::cout << "Arguments:" << std::endl;
    for (int i = 1; i < argc; ++i) {
        std::cout << "Argument " << i << ": " << argv[i] << std::endl;
        fileList.emplace_back(argv[i]);
    }

    return 0;
}