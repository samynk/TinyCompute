#pragma once

#include <clang/Tooling/Tooling.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/DiagnosticIDs.h>

#include <unordered_set>
#include "PendingEdit.h"

enum class GLSLDataType {
	BOOL, INT, UINT, FLOAT, DOUBLE, OTHER
};

class KernelRewriter : public clang::RecursiveASTVisitor<KernelRewriter> {
public:
	KernelRewriter(clang::ASTContext* pContext, std::vector<PendingEdit>& edits);
	bool VisitFieldDecl(clang::FieldDecl* pField);
	bool VisitFunctionDecl(clang::FunctionDecl* pFunction);
	bool VisitVarDecl(clang::VarDecl* pVar);
	bool VisitDeclRefExpr(clang::DeclRefExpr* pRef);
	bool VisitImplicitCastExpr(clang::ImplicitCastExpr* pImplicitCast);

	static void focusedDump(const clang::Stmt* S, clang::ASTContext& Ctx);
	
	bool VisitCXXConstructExpr(clang::CXXConstructExpr* pConstructorCall);
	bool VisitCXXFunctionalCastExpr(clang::CXXFunctionalCastExpr* pCastEpr);

	clang::SourceLocation afterLSquare(const clang::Expr* Base);
    clang::SourceLocation afterRSquare(const clang::Expr* Index);

	bool VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr* E);

	bool TraverseFieldDecl(clang::FieldDecl* fieldDec) {
		return this->WalkUpFromFieldDecl(fieldDec);
	}
private:
	static void dumpDyn(const clang::Expr* E, llvm::StringRef label) {
		if (!E) { llvm::errs() << label << ": <null>\n"; return; }
		std::string stmClassName = E->getStmtClassName();
		llvm::errs() << label << ": " << stmClassName
			<< "  type=" << E->getType().getAsString() << "\n";
	}

	static std::optional<std::string> glslTypeForVecBase(clang::QualType qt);
	static std::optional<std::string> glslTypeForElement(clang::QualType elemType);
	void rewriteVecCtorType(const clang::Expr* E);
	GLSLDataType getGLSLDataType(clang::QualType elemType);
	std::string getGLSLDataTypeAsString(GLSLDataType);
	unsigned int getGSLDataTypeRank(GLSLDataType);
	void castTo(clang::Expr* pExpr, GLSLDataType targetCast);
	bool checkBufferBinding(const clang::FieldDecl* pField);
	bool rewriteBufferBinding(const clang::FieldDecl* pField);
	bool checkImageBinding(const clang::FieldDecl* pField);
	bool rewriteImageBinding(const clang::FieldDecl* pField);
	bool checkUniformField(clang::FieldDecl* pField);
	bool rewriteUniform(const clang::FieldDecl* pField);
	
	clang::ASTContext* m_pASTContext;
	std::vector<PendingEdit>& m_PendingEdits;

	std::unordered_set<std::string> m_PredefinedVariables = {
		"gl_NumWorkGroups",
		"gl_WorkGroupID",
		"gl_LocalInvocationID",
		"gl_GlobalInvocationID",
		"gl_LocalInvocationIndex"
	};
};
