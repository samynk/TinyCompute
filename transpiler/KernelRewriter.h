#pragma once

#include <clang/Tooling/Tooling.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/DiagnosticIDs.h>

#include <unordered_set>
#include <map>
#include "PendingEdit.h"
#include "ImageFormatDescriptor.h"

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
	bool VisitCallExpr(clang::CallExpr* callExpr);
	bool VisitUsingDirectiveDecl(clang::UsingDirectiveDecl* pUsing);

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
	static std::string typeNameNoScope(clang::QualType QT,const clang::ASTContext& context) {
		clang::PrintingPolicy pol(context.getLangOpts());
		pol.SuppressScope = true;           // <-- kill "ns::"
		pol.SuppressUnwrittenScope = true;  // drop inline/anon scopes
		pol.SuppressTagKeyword = true;      // drop "struct/class/enum" prefixes
	
		return QT.getAsString(pol);
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

	bool rewriteField(const clang::FieldDecl* pField);

	std::optional<std::string> getUnqualifiedEnumType(const clang::TemplateArgument& ta);
	bool isInNamespace(const clang::NamedDecl* FD, llvm::StringRef NS);

	clang::ASTContext* m_pASTContext;
	std::vector<PendingEdit>& m_PendingEdits;

	std::unordered_set<std::string> m_PredefinedVariables = {
		"gl_NumWorkGroups",
		"gl_WorkGroupID",
		"gl_LocalInvocationID",
		"gl_GlobalInvocationID",
		"gl_LocalInvocationIndex"
	};

	inline static const std::map<std::string, ImageFormatDescriptor> m_ImageFormats =
	{
		// floating point formats
		{"RGBA32F", {"rgba32f",tc::Scalar::FLOAT} },
		{"RGBA16F", {"rgba16f",tc::Scalar::FLOAT}},
		{"RG32F",{"rg32f",tc::Scalar::FLOAT}},
		{"RG16F",{"rg16f",tc::Scalar::FLOAT}},
		{"R11F_G11F_B10F",{"r11f_g11f_b10f",tc::Scalar::FLOAT}},
		{"R32F",{"r32f",tc::Scalar::FLOAT}},
		{"R16F",{"r16f",tc::Scalar::FLOAT}},
		// unorm formats
		{"RGBA16",{	"rgba16",tc::Scalar::UNORM}},
		{"RGB10_A2",{"rgb10_a2",tc::Scalar::UNORM}},
		{"RGBA8",{"rgba8",tc::Scalar::UNORM}},
		{"RG16",{"rg16",tc::Scalar::UNORM}},
		{"RG8",{"rg8",tc::Scalar::UNORM}},
		{"R16",{"r16",tc::Scalar::UNORM}},
		{"R8",{"r8",tc::Scalar::UNORM}},
		// signed normal formats
		{"RGBA16_SNORM",{"rgba16_snorm",tc::Scalar::SNORM}},
		{"RGBA8_SNORM",{"rgba8_snorm",tc::Scalar::SNORM}},
		{"RG16_SNORM",{"rg16_snorm",tc::Scalar::SNORM}},
		{"RG8_SNORM",{"rg8_snorm",tc::Scalar::SNORM}},
		{"R16_SNORM",{"r16_snorm",tc::Scalar::SNORM}},
		// unsigned integer formats
		{"RGBA32UI",{"rgba32ui",tc::Scalar::UINT}},
		{"RGBA16UI",{"rgba16ui",tc::Scalar::UINT}},
		{"RGB10_A2UI",{"rgb10_a2ui",tc::Scalar::UINT}},
		{"RGBA8UI",{"rgba8ui",tc::Scalar::UINT}},
		{"RG32UI",{"rg32ui",tc::Scalar::UINT}},
		{"RG16UI",{"rg16ui",tc::Scalar::UINT}},
		{"RG8UI",{"rg8ui",tc::Scalar::UINT}},
		{"R32UI",{"r32ui",tc::Scalar::UINT}},
		{"R16UI",{"r16ui",tc::Scalar::UINT}},
		{"R8UI", {"r8ui",tc::Scalar::UINT}},
		//signed integer formats
		{ "RGBA32I", {"rgba32i",tc::Scalar::INT} },
		{ "RGBA16I", {"rgba16i",tc::Scalar::INT} },
		{ "RGBA8I", {"rgba8i",tc::Scalar::INT} },
		{ "RG32I", {"rg32i",tc::Scalar::INT} },
		{ "RG16I", {"rg16i",tc::Scalar::INT} },
		{ "RG8I", {"rg8i",tc::Scalar::INT} },
		{ "R32I", {"r32i",tc::Scalar::INT} },
		{ "R16I", {"r16i",tc::Scalar::INT} },
		{ "R8I", {"r8i",tc::Scalar::INT} }
	};

	inline static const std::map<std::string, std::string> m_DimensionSuffix =
	{
		{"D2","2D"},{"D3","3D"},{"Cube", "Cube"}
	};

	inline static const std::map<tc::Scalar, std::string> m_TypePrefix =
	{
		{tc::Scalar::FLOAT, ""},
		{tc::Scalar::INT, "i"},
		{tc::Scalar::UINT, "u"},
		{tc::Scalar::UNORM, ""},
		{tc::Scalar::SNORM, ""}
	};
};
