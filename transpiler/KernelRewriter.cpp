#include "KernelRewriter.h"

#include <clang/astmatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/AST/TextNodeDumper.h> 

KernelRewriter::KernelRewriter(clang::ASTContext* pContext, std::vector<PendingEdit>& edits)
	:m_pASTContext(pContext), m_PendingEdits{ edits }
{
}

std::optional<std::string> KernelRewriter::glslTypeForVecBase(clang::QualType qt) {
	// Normalize: drop refs/cv and typedef sugar
	using namespace clang;
	qt = qt.getNonReferenceType().getUnqualifiedType().getCanonicalType();

	const auto* RT = qt->getAs<RecordType>();
	if (!RT) return std::nullopt;

	const auto* RD = RT->getAsCXXRecordDecl();
	if (!RD) return std::nullopt;

	const auto* Spec = llvm::dyn_cast<ClassTemplateSpecializationDecl>(RD);
	if (!Spec) return std::nullopt;

	// Make sure it is tc::vec_base<..., ...>
	// (compare the primary template’s qualified name)
	const auto* Prim = Spec->getSpecializedTemplate();
	if (!Prim || Prim->getQualifiedNameAsString() != "tc::vec_base")
		return std::nullopt;

	const auto& args = Spec->getTemplateArgs().asArray();
	if (args.size() != 2) return std::nullopt;
	if (args[0].getKind() != TemplateArgument::Type ||
		args[1].getKind() != TemplateArgument::Integral)
		return std::nullopt;

	QualType elemT = args[0].getAsType();
	unsigned N = static_cast<unsigned>(args[1].getAsIntegral().getLimitedValue());

	if (N < 2 || N > 4) return std::nullopt; // your VecSize constraint

	auto scalar = glslTypeForElement(elemT);
	if (!scalar) return std::nullopt;

	std::string prefix;
	if (*scalar == "float")  prefix = "vec";
	else if (*scalar == "double") prefix = "dvec";
	else if (*scalar == "int")    prefix = "ivec";
	else if (*scalar == "uint")   prefix = "uvec";
	else if (*scalar == "bool")   prefix = "bvec";
	else return std::nullopt;

	return prefix + std::to_string(N);
}

std::optional<std::string> KernelRewriter::glslTypeForElement(clang::QualType elemType)
{
	clang::QualType qElemType = elemType.getCanonicalType();
	if (qElemType->isBooleanType()) return std::string("bool");
	if (qElemType->isUnsignedIntegerType()) { return std::string("uint"); }
	if (qElemType->isIntegerType()) return std::string("int");
	if (qElemType->isFloatingType()) return std::string("float");
	if (qElemType->isDoubleType()) return std::string("double");
	else {
		//elemType
		return glslTypeForVecBase(elemType);
	}

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



bool KernelRewriter::rewriteBufferBinding(const clang::FieldDecl* FD) {
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
			if (!glslElemTypeOpt) {
				return true;
			}
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
	if (checkBufferBinding(pField)) {
		rewriteBufferBinding(pField);
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

bool KernelRewriter::checkBufferBinding(const clang::FieldDecl* pField) {
	using namespace clang::ast_matchers;
	auto bindingPointMatcher = fieldDecl(
		hasType(qualType(hasDeclaration(
			classTemplateSpecializationDecl(hasName("BufferBinding"))
		)))
	).bind("bufferBinding");

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

			llvm::errs() << "Converting uniform type\n";
			auto glslElemTypeOpt = glslTypeForElement(elemType);
			if (!glslElemTypeOpt) {
				return true;
			}
			std::string glslElemType = *glslElemTypeOpt;
			llvm::errs() << "Type is " << glslElemType << "\n";
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



bool KernelRewriter::rewriteImageBinding(const clang::FieldDecl* FD)
{
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
			if (!glslElemTypeOpt) {
				return true;
			}
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

bool KernelRewriter::checkImageBinding(const clang::FieldDecl* pField)
{
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
		auto typeLoc = pVar->getTypeSourceInfo();
		if (typeLoc) {
			clang::SourceRange typeRange = typeLoc->getTypeLoc().getSourceRange();
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
	
	clang::SourceRange sr = pExpr->IgnoreParenImpCasts()->getSourceRange();

	// Convert the enum to string
	std::string castStr;
	switch (targetCast) {
	case GLSLDataType::BOOL:   castStr = "bool"; break;
	case GLSLDataType::INT:    castStr = "int"; break;
	case GLSLDataType::UINT:   castStr = "uint"; break;
	case GLSLDataType::FLOAT:  castStr = "float"; break;
	case GLSLDataType::DOUBLE: castStr = "double"; break;
	}

	llvm::errs() << "Implicit cast to " << castStr << "\n";
	pExpr->dump();

	clang::SourceManager& sourceManager = m_pASTContext->getSourceManager();
	const clang::LangOptions& languageOptions = m_pASTContext->getLangOpts();

	auto exprText = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(sr), sourceManager, languageOptions);
	llvm::errs() << "Expression text :" << exprText << "\n";

	auto insertBefore = castStr + "( ";
	auto insertAfter = " )";

	// Insert "cast(" before the expression
	PendingEdit beforeCast{
		clang::SourceRange{sr.getBegin(),sr.getBegin()},
		insertBefore,true
	};

	clang::SourceLocation endLoc = clang::Lexer::getLocForEndOfToken(
		sr.getEnd(), 0, sourceManager, languageOptions);

	PendingEdit afterCast{
		clang::SourceRange{endLoc,endLoc},
		insertAfter,true,true
	};


	m_PendingEdits.emplace_back(beforeCast);
	m_PendingEdits.emplace_back(afterCast);
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

	/*llvm::errs() << "Warning: Implicit cast from "
		<< sourceType.getAsString() << " to "
		<< targetType.getAsString() << "\n";*/

	if (src != dst && ((srcRank > toRank) || src == GLSLDataType::BOOL || dst == GLSLDataType::BOOL))
	{
		// Only warn or edit if GLSL would disallow it
		if (src != GLSLDataType::OTHER && dst != GLSLDataType::OTHER) {


			this->castTo(subExpr, dst);  // Surround the subexpression with explicit cast
		}
	}
	return true;
}

void KernelRewriter::focusedDump(const clang::Stmt* S, clang::ASTContext& Ctx) {
	clang::TextNodeDumper D(llvm::errs(), Ctx,
		/*ShowColors=*/true);
	D.Visit(S);  // also works for Decl*, Type, TypeLoc
}



void KernelRewriter::rewriteVecCtorType(const clang::Expr* E) {
	if (!E) return;
	//focusedDump(E,*m_pASTContext);
	using namespace clang;
	if (auto* TO = dyn_cast<CXXTemporaryObjectExpr>(E->IgnoreParenImpCasts())) {
		if (auto* TSI = TO->getTypeSourceInfo()) {
			llvm::errs() << "TSI != null :" << TSI->getType() << "\n ";
			QualType QT = TSI->getType();
			if (auto glsl = glslTypeForVecBase(QT)) {
				llvm::errs() << "glsl type : " << glsl << "\n";
				TypeLoc TL = TSI->getTypeLoc();
				llvm::errs() << "adding edit \n";
				PendingEdit edit{ TL.getSourceRange(),glsl.value()};
				m_PendingEdits.emplace_back(edit);
			}
		}
		return;
	}

	if (auto* FC = dyn_cast<CXXFunctionalCastExpr>(E->IgnoreParenImpCasts())) {
		if (auto* TSI = FC->getTypeInfoAsWritten()) {
			QualType QT = TSI->getType();
			if (auto glsl = glslTypeForVecBase(QT)) {
				TypeLoc TL = TSI->getTypeLoc();
				llvm::errs() << "functional cast glsl type : " << glsl << "\n";
				//PendingEdit edit{ TL.getSourceRange(),*glsl };
				//m_PendingEdits.emplace_back(edit);
			}
		}
		return;
	}
}


bool KernelRewriter::VisitCXXConstructExpr(clang::CXXConstructExpr* pConstructorCall)
{
	rewriteVecCtorType(pConstructorCall);
	return true;
}

bool KernelRewriter::VisitCXXFunctionalCastExpr(clang::CXXFunctionalCastExpr* pCastExpr)
{
	rewriteVecCtorType(pCastExpr);
	return true;
}

clang::SourceLocation KernelRewriter::afterLSquare(const clang::Expr* Base) {
	return clang::Lexer::findLocationAfterToken(Base->getEndLoc(), clang::tok::l_square,
		m_pASTContext->getSourceManager(), m_pASTContext->getLangOpts(),
		/*SkipTrailingWhitespaceAndNewLine=*/true);
}

clang::SourceLocation KernelRewriter::afterRSquare(const clang::Expr* Index) {
	return clang::Lexer::findLocationAfterToken(Index->getEndLoc(), clang::tok::r_square,
		m_pASTContext->getSourceManager(), m_pASTContext->getLangOpts(),
		/*SkipTrailingWhitespaceAndNewLine=*/true);
}

bool KernelRewriter::VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr* E) {
	using namespace clang;
	if (E->getOperator() != OO_Subscript) return true;

	llvm::errs() << "found array index\n";

	auto* UDL = dyn_cast<clang::UserDefinedLiteral>(E->getArg(1)->IgnoreParenImpCasts());
	if (!UDL) return true;

	auto* ID = UDL->getUDSuffix();
	llvm::errs() << "suffix " << ID->getName() << "\n";
	if (!ID || ID->getName() != "_sw") return true;

	auto& SM = m_pASTContext->getSourceManager();
	auto& LO = m_pASTContext->getLangOpts();
	std::string swizzle;
	CharSourceRange swizzleRange = CharSourceRange::getTokenRange(UDL->getSourceRange());
	auto txt = Lexer::getSourceText(swizzleRange, SM, LO);
	auto l = txt.find('"'), r = (l == StringRef::npos) ? StringRef::npos : txt.find('"', l + 1);
	if (l != StringRef::npos && r != StringRef::npos)
	{
		swizzle = txt.substr(l + 1, r - l - 1).str();
	}


	// extract swizzle string
	if (swizzle.empty()) {
		return true;
	}

	std::string repl = "." + swizzle;
	llvm::errs() << "Replacing with " << repl << "\n";

	SourceLocation beginLoc = Lexer::getLocForEndOfToken(
		E->getArg(0)->getEndLoc(), 0, SM, LO);

	SourceRange range = { beginLoc,E->getEndLoc() };
	PendingEdit cast{
		range,
		repl
	};

	m_PendingEdits.emplace_back(cast);

	return true;
}
