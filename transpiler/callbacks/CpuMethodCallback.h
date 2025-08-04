#pragma once
#include <clang/astmatchers/ASTMatchFinder.h>
#include <clang/AST/Type.h>
#include <clang/AST/Expr.h>
#include <clang/AST/TemplateBase.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Lex/Lexer.h>

#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/lex/Lexer.h>

#include <vector>
#include "../PendingEdit.h"


using namespace clang;
using namespace clang::ast_matchers;
class CpuMethodCallback : public MatchFinder::MatchCallback {
public:
	CpuMethodCallback(std::vector<PendingEdit>& edits) :m_PendingEdits(edits) {}

	void run(const MatchFinder::MatchResult& Result) override {
		if (const CXXMethodDecl* cpuMethod = Result.Nodes.getNodeAs<CXXMethodDecl>("cpuMethod"))
		{
			llvm::errs() << "Found cpu method:" << cpuMethod->getNameAsString() << "\n";
			PendingEdit edit{cpuMethod->getSourceRange(),""};
			m_PendingEdits.emplace_back(edit);
		}
	}
private:
	std::vector<PendingEdit>& m_PendingEdits;
};