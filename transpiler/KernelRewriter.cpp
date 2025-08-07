#include "KernelRewriter.h"

#include <clang/astmatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

KernelRewriter::KernelRewriter(clang::ASTContext* pContext, std::vector<PendingEdit>& edits)
	:m_pASTContext(pContext), m_PendingEdits{ edits }
{
}

std::optional<std::string> KernelRewriter::glslTypeForElement(clang::QualType elemType) {
	elemType = elemType.getCanonicalType();
	if (elemType->isBooleanType()) return std::string("bool");
	if (elemType->isUnsignedIntegerType()) { return std::string("uint"); }
	if (elemType->isIntegerType()) return std::string("int");
	if (elemType->isFloatingType()) return std::string("float");
	if (elemType->isDoubleType()) return std::string("double");
	return std::nullopt;
}

GLSLDataType KernelRewriter::getGLSLDataType(clang::QualType elemType)
{
	elemType = elemType.getCanonicalType();
	if (elemType->isBooleanType()) return GLSLDataType::BOOL;
	if (elemType->isUnsignedIntegerType()) return GLSLDataType::UINT;
	if (elemType->isIntegerType()) return GLSLDataType::INT;
	if (elemType->isDoubleType()) return GLSLDataType::DOUBLE;
	if (elemType->isFloatingType()) return GLSLDataType::FLOAT;
	return GLSLDataType::OTHER;
}

std::string KernelRewriter::getGLSLDataTypeAsString(GLSLDataType type)
{
	switch (type) {
	case GLSLDataType::BOOL: return "bool";
	case GLSLDataType::INT: return "int";
	case GLSLDataType::UINT: return "uint";
	case GLSLDataType::FLOAT: return "float";
	case GLSLDataType::DOUBLE: return "double";
	default:return "other";
	}
}

unsigned int KernelRewriter::getGSLDataTypeRank(GLSLDataType type)
{
	switch (type) {
	case GLSLDataType::BOOL: return 0;
	case GLSLDataType::INT: return 1;
	case GLSLDataType::UINT: return 2;
	case GLSLDataType::FLOAT: return 3;
	case GLSLDataType::DOUBLE: return 4;
	default:return 0;
	}
}



bool KernelRewriter::rewriteBindingPoint(const clang::FieldDecl* FD) {
	using namespace clang;
	const SourceManager& SM = m_pASTContext->getSourceManager();

	const QualType QT = FD->getType();
	const TemplateSpecializationType* TST = QT->getAs<TemplateSpecializationType>();
	if (!TST) return true;

	QualType elemType;
	unsigned binding = 0, set = 0;

	if (const auto* CTSDecl = dyn_cast<ClassTemplateSpecializationDecl>(
		TST->getAsRecordDecl())) {

		const auto& Args = CTSDecl->getTemplateArgs();
		if (Args.size() >= 3) {
			// Elem type is Arg 0
			QualType elemType = Args[0].getAsType();

			// binding is Arg 1
			unsigned binding = 0;
			if (Args[1].getKind() == clang::TemplateArgument::ArgKind::Integral) {
				binding = static_cast<unsigned>(Args[1].getAsIntegral().getZExtValue());
			}

			// set is Arg 2
			unsigned set = 0;
			if (Args[2].getKind() == clang::TemplateArgument::ArgKind::Integral) {
				set = static_cast<unsigned>(Args[2].getAsIntegral().getZExtValue());
			}

			auto glslElemTypeOpt = glslTypeForElement(elemType);
			std::string glslElemType = *glslElemTypeOpt;

			std::string varName = FD->getNameAsString();
			// Build GLSL buffer declaration (SSBO style)
			std::string glsl = "layout(set = " + std::to_string(set) +
				", binding = " + std::to_string(binding) + ") buffer " + "_" + varName + "Layout" + " {"
				+ glslElemType + " " + varName + "[]; }; ";

			llvm::errs() << "Replacing binding with :\n" << glsl << "\n.";

			// Replace the entire field declaration (including initializer)
			SourceLocation endLoc = Lexer::getLocForEndOfToken(
				FD->getSourceRange().getEnd(), 0, SM, m_pASTContext->getLangOpts());
			SourceRange fullRange(FD->getSourceRange().getBegin(), endLoc);

			PendingEdit edit{ fullRange, glsl };
			m_PendingEdits.emplace_back(edit);
			return false;
		}
		else {
			return true;
		}
		
	}
	return true;
}

bool KernelRewriter::VisitFieldDecl(clang::FieldDecl* pField)
{
	if (checkBindingPoint(pField)) {
		rewriteBindingPoint(pField);
	}
	else if (checkUniformField(pField)) {
		rewriteUniform(pField);
	}
	return true;
}

bool KernelRewriter::VisitFunctionDecl(clang::FunctionDecl* pFunction)
{
	if (pFunction->getNameAsString().starts_with("_")) {
		return false;
	}
	clang::QualType returnType = pFunction->getReturnType();
	auto changedType = this->glslTypeForElement(returnType);
	if (changedType.has_value())
	{

		PendingEdit edit{ pFunction->getReturnTypeSourceRange(),changedType.value() };
		m_PendingEdits.emplace_back(edit);
	}
	return true;
}

bool KernelRewriter::checkBindingPoint(const clang::FieldDecl* pField) {
	using namespace clang::ast_matchers;
	auto bindingPointMatcher = fieldDecl(
		hasType(qualType(hasDeclaration(
			classTemplateSpecializationDecl(hasName("BindingPoint"))
		)))
	).bind("bindingPoint");

	auto innerMatches = match(
		bindingPointMatcher,
		*pField,
		*m_pASTContext
	);
	if (!innerMatches.empty())
	{
		return true;
	}
	else {
		return false;
	}
}

bool KernelRewriter::rewriteUniform(const clang::FieldDecl* FD) {
	using namespace clang;
	const SourceManager& SM = m_pASTContext->getSourceManager();

	const QualType QT = FD->getType();
	const TemplateSpecializationType* TST = QT->getAs<TemplateSpecializationType>();
	if (!TST) return true;

	QualType elemType;
	unsigned location = 0, set = 0;

	if (const auto* CTSDecl = dyn_cast<ClassTemplateSpecializationDecl>(
		TST->getAsRecordDecl())) {

		const auto& Args = CTSDecl->getTemplateArgs();
		if (Args.size() >= 2) {
			// Elem type is Arg 0
			QualType elemType = Args[0].getAsType();
			auto glslElemTypeOpt = glslTypeForElement(elemType);
			std::string glslElemType = *glslElemTypeOpt;
			// binding is Arg 1
			unsigned location = 0;
			if (Args[1].getKind() == clang::TemplateArgument::ArgKind::Integral) {
				location = static_cast<unsigned>(Args[1].getAsIntegral().getZExtValue());
			}


			std::string varName = FD->getNameAsString();
			std::string name = FD->getNameAsString();
			std::string uniformDecl = "layout(location = " + std::to_string(location) +
				") uniform " + glslElemType + " " + name + ";";

			llvm::errs() << "Replacing location with :\n" << uniformDecl << "\n.";

			// Replace the entire field declaration (including initializer)
			SourceLocation endLoc = Lexer::getLocForEndOfToken(
				FD->getSourceRange().getEnd(), 0, SM, m_pASTContext->getLangOpts());
			SourceRange fullRange(FD->getSourceRange().getBegin(), endLoc);

			// Do the replacement
			PendingEdit edit{ fullRange, uniformDecl };
			m_PendingEdits.emplace_back(edit);
			return false;
		}
	}
	return true;
}

bool KernelRewriter::checkUniformField(clang::FieldDecl* pField) {
	using namespace clang::ast_matchers;
	auto uniformMatcher = fieldDecl(
		hasType(qualType(hasDeclaration(
			classTemplateSpecializationDecl(hasName("Uniform"))
		)))
	);

	auto innerMatches = match(
		uniformMatcher,
		*pField,
		*m_pASTContext
	);
	if (!innerMatches.empty())
	{
		return true;
	}
	else {
		return false;
	}

}

bool KernelRewriter::VisitVarDecl(clang::VarDecl* pVar) {
	clang::QualType type = pVar->getType();

	auto changedType = this->glslTypeForElement(type);
	if (changedType.has_value())
	{
		/*llvm::errs() << "VisitVarDecl : " << pVar->getNameAsString() << "\n";
		clang::SourceRange typeRange{ pVar->getTypeSpecStartLoc(),pVar->getTypeSpecEndLoc() };
		PendingEdit edit{ typeRange,changedType.value() };
		m_PendingEdits.emplace_back(edit);*/

		auto typeLoc = pVar->getTypeSourceInfo();
		if (typeLoc) {
			clang::SourceRange typeRange = typeLoc->getTypeLoc().getSourceRange();

			// Extra check: is the range valid?
			if (typeRange.isValid()) {
				llvm::errs() << "VisitVarDecl : " << pVar->getNameAsString() << "\n";

				PendingEdit edit{ typeRange, changedType.value() };
				m_PendingEdits.emplace_back(edit);
			}
		}
	}

	return true;
}

bool KernelRewriter::VisitDeclRefExpr(clang::DeclRefExpr* pRef)
{
	clang::NestedNameSpecifier* ns = pRef->getQualifier();
	if (ns != nullptr)
	{
		if (pRef->getDecl() != nullptr) {
			llvm::errs() << "DeclRefExpr : " << pRef->getDecl()->getNameAsString() << "\n";

			auto qualifierRange = pRef->getQualifierLoc().getSourceRange();
			PendingEdit removeNS{ qualifierRange,"" };
			m_PendingEdits.emplace_back(removeNS);
		}
	}
	return true;
}

void KernelRewriter::castTo(clang::Expr* pExpr, GLSLDataType targetCast)
{
	if (targetCast == GLSLDataType::OTHER)
	{
		return;
	}
	clang::SourceRange sr = pExpr->getSourceRange();

	// Convert the enum to string
	std::string castStr;
	switch (targetCast) {
	case GLSLDataType::BOOL:   castStr = "bool"; break;
	case GLSLDataType::INT:    castStr = "int"; break;
	case GLSLDataType::UINT:   castStr = "uint"; break;
	case GLSLDataType::FLOAT:  castStr = "float"; break;
	case GLSLDataType::DOUBLE: castStr = "double"; break;
	}

	llvm::errs() << "Casting to " << castStr << "\n";

	clang::SourceManager& sourceManager = m_pASTContext->getSourceManager();
	const clang::LangOptions& languageOptions = m_pASTContext->getLangOpts();

	auto exprText = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(sr), sourceManager, languageOptions);
	

	// Insert "cast(" before the expression
	PendingEdit cast{
		clang::SourceRange{sr.getBegin(),sr.getEnd()},
		castStr + "( " + exprText.str() + " )"
	};

	m_PendingEdits.emplace_back(cast);

}

bool KernelRewriter::VisitImplicitCastExpr(clang::ImplicitCastExpr* pImplicitCast)
{
	auto subExpr = pImplicitCast->getSubExpr();

	// Get the source type and target type
	clang::QualType sourceType = subExpr->getType()->getCanonicalTypeUnqualified();
	clang::QualType targetType = pImplicitCast->getType()->getCanonicalTypeUnqualified();

	GLSLDataType src = getGLSLDataType(sourceType);
	GLSLDataType dst = getGLSLDataType(targetType);
	unsigned srcRank = getGSLDataTypeRank(src);
	unsigned toRank = getGSLDataTypeRank(dst);

	llvm::errs() << "Warning: Implicit cast from "
		<< sourceType.getAsString() << " to "
		<< targetType.getAsString() << "\n";

	if (src != dst  && ((srcRank > toRank) || src == GLSLDataType::BOOL || dst == GLSLDataType::BOOL)) 
	{
		// Only warn or edit if GLSL would disallow it
		if ( src != GLSLDataType::OTHER && dst != GLSLDataType::OTHER) {
			

			this->castTo(subExpr, dst);  // Surround the subexpression with explicit cast
		}
	}
	return true;
}
