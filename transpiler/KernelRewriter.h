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

	bool TraverseTemplateArgument(const clang::TemplateArgument pTemplateArguments)
	{
		return false;
	}
	bool VisitClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl* pDecl)
	{
		return false;
	}
private:
	std::optional<std::string> glslTypeForElement(clang::QualType elemType);
	GLSLDataType getGLSLDataType(clang::QualType elemType);
	std::string getGLSLDataTypeAsString(GLSLDataType);
	unsigned int getGSLDataTypeRank(GLSLDataType);
	void castTo(clang::Expr* pExpr, GLSLDataType targetCast);
	bool checkBindingPoint(const clang::FieldDecl* pField);
	bool rewriteBindingPoint(const clang::FieldDecl* pField);
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
