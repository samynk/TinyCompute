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

std::optional<std::string> KernelRewriter::getUnqualifiedEnumType(const clang::TemplateArgument& ta) {
	using namespace clang;
	llvm::APSInt val = ta.getAsIntegral();
	QualType T = ta.getIntegralType();
	if (const EnumType* ET = T->getAs<EnumType>()) {
		const EnumDecl* ED = ET->getDecl();
		for (const EnumConstantDecl* E : ED->enumerators()) {
			if (E->getInitVal() == val) {
				return E->getName().str();
			}
		}
	}
	return {};
}

bool KernelRewriter::isInNamespace(const clang::NamedDecl* FD, llvm::StringRef NS)
{
	const clang::DeclContext* DC = FD->getDeclContext();
	while (DC) {
		if (auto* ND = llvm::dyn_cast<clang::NamespaceDecl>(DC)) {
			if (ND->getName() == NS) return true; // handles nested: climb outward
		}
		DC = DC->getParent();
	}
	return false;
}

std::string KernelRewriter::printTypeNoNS(clang::QualType& qt)
{
	clang::PrintingPolicy policy(m_pASTContext->getLangOpts());
	policy.SuppressScope = true;
	policy.SuppressUnwrittenScope = true;
	return qt.getAsString(policy);
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
			std::string glslElemType;
			if (glslElemTypeOpt) {
				glslElemType = *glslElemTypeOpt;
			}
			else {
				glslElemType = typeNameNoScope(elemType, *m_pASTContext);
			}

			std::string varName = FD->getNameAsString();
			// Build GLSL buffer declaration (SSBO style)
			std::string glsl = "layout(set = " + std::to_string(set) +
				", binding = " + std::to_string(binding) + ") buffer " + "_" + varName + "Layout" + " {"
				+ glslElemType + " " + varName + "[]; }; ";
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
	else if (checkImageBinding(pField)) {
		rewriteImageBinding(pField);
	}
	else if (checkUniformField(pField)) {
		rewriteUniform(pField);
	}
	else if (checkStdArrayField(pField)) {
		rewriteStdArray(pField);
	}
	else {
		// remove namespaces of fields.
		rewriteField(pField);
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

			auto glslElemTypeOpt = glslTypeForElement(elemType);
			if (!glslElemTypeOpt) {
				return true;
			}
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

bool KernelRewriter::checkStdArrayField(const clang::FieldDecl* pField)
{
	using namespace clang::ast_matchers;
	auto m = fieldDecl(
		hasType(
			classTemplateSpecializationDecl(
				hasName("::std::array")
			)
		)
		//hasInitializer(initListExpr())
	);
	auto innerMatches = match(
		m,
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

bool KernelRewriter::rewriteStdArray(const clang::FieldDecl* pField)
{
	using namespace clang;
	const QualType QT = pField->getType();
	const TemplateSpecializationType* TST = QT->getAs<TemplateSpecializationType>();
	if (!TST) return true;

	QualType elemType;
	unsigned location = 0, set = 0;

	if (const auto* CTSDecl = dyn_cast<ClassTemplateSpecializationDecl>(
		TST->getAsRecordDecl())) {

		const auto& Args = CTSDecl->getTemplateArgs();
		QualType elemType = Args[0].getAsType();

		unsigned int size = 0;
		if (Args[1].getKind() == clang::TemplateArgument::ArgKind::Integral) {
			size = static_cast<unsigned>(Args[1].getAsIntegral().getZExtValue());
		}
		auto glslType = glslTypeForElement(elemType);
		std::string arrayType;
		if (glslType)
		{
			arrayType = glslType.value();
		}
		else {
			arrayType = elemType.getAsString();
		}

		std::string replacement = "const "
			+ arrayType + " "
			+ pField->getDeclName().getAsString()
			+ "[" + std::to_string(size) + "] = "
			+ arrayType + "[" + std::to_string(size) + "]";

		SourceManager& sm = m_pASTContext->getSourceManager();
		clang::SourceLocation B = sm.getSpellingLoc(pField->getLocation());
		clang::SourceLocation E = clang::Lexer::getLocForEndOfToken(B, 1, sm, m_pASTContext->getLangOpts());

		SourceRange sr{ pField->getBeginLoc(),E };
		PendingEdit pe{ sr, replacement };

		m_PendingEdits.emplace_back(pe);
	}

	return true;
}

bool KernelRewriter::VisitInitListExpr(clang::InitListExpr* pInitList)
{
	using namespace clang;
	

	pInitList->dump();
	SourceLocation bl = pInitList->getLBraceLoc();
	SourceLocation br = pInitList->getRBraceLoc();

	if (pInitList->isSemanticForm()) {
		return true;
	}

	if (bl.isInvalid() || br.isInvalid()) {
		llvm::errs() << "Invalid braces\n";
		return true;
	}
	
	PendingEdit pe{ bl ,"(" };
	m_PendingEdits.emplace_back(pe);

	
	PendingEdit pe2{ br,")" };
	m_PendingEdits.emplace_back(pe2);
	return true;
}

bool KernelRewriter::VisitCXXMemberCallExpr(clang::CXXMemberCallExpr* pMemberCall)
{
	using namespace clang::ast_matchers;
	auto sizeMatcher = cxxMemberCallExpr(
		callee(cxxMethodDecl(
			hasName("size"),
			ofClass(classTemplateSpecializationDecl(
				hasName("std::array")
			).bind("arrSpec"))
		))
	).bind("call");

	auto innerMatches = match(
		sizeMatcher,
		*pMemberCall,
		*m_pASTContext
	);
	if (!innerMatches.empty())
	{
		const auto* arrSpec =
			innerMatches[0].getNodeAs<clang::ClassTemplateSpecializationDecl>("arrSpec");
		const auto& args = arrSpec->getTemplateArgs();
		if (args.size() < 2) return true;

		if (args[1].getKind() == clang::TemplateArgument::ArgKind::Integral) {
			auto size = static_cast<unsigned>(args[1].getAsIntegral().getZExtValue());
			PendingEdit pe{ pMemberCall->getSourceRange(), std::to_string(size) };
			m_PendingEdits.emplace_back(pe);
		}
		return true;
	}
	return true;
}

bool KernelRewriter::rewriteField(const clang::FieldDecl* pField)
{
	using namespace clang;
	if (pField->getNameAsString() == "local_size")
	{
		return true;
	}
	if (auto* TSI = pField->getTypeSourceInfo()) {
		TypeLoc TL = TSI->getTypeLoc();

		// Many qualified types appear as ElaboratedTypeLoc
		if (auto ETL = TL.getAs<ElaboratedTypeLoc>()) {
			if (auto Q = ETL.getQualifierLoc()) {
				auto QRange = Q.getSourceRange();
				PendingEdit edit{ QRange, "" };
				m_PendingEdits.emplace_back(edit);
				return true;
			}
		}

		// Also handle dependent/template spellings that carry a qualifier
		if (auto DT = TL.getAs<DependentNameTypeLoc>()) {
			if (auto Q = DT.getQualifierLoc()) {
				PendingEdit edit{ Q.getSourceRange(), "" };
				m_PendingEdits.emplace_back(edit);
				return true;
			}
		}
		if (auto DTS = TL.getAs<DependentTemplateSpecializationTypeLoc>()) {
			if (auto Q = DTS.getQualifierLoc()) {
				PendingEdit edit{ Q.getSourceRange(), "" };
				m_PendingEdits.emplace_back(edit);
				return true;
			}
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

			auto imageFormat = getUnqualifiedEnumType(Args[0]);
			if (!imageFormat) {
				return true;
			}
			auto dimension = getUnqualifiedEnumType(Args[1]);
			if (!dimension) {
				return true;
			}
			// binding is Arg 4
			unsigned binding = 0;
			if (Args.size() >= 4)
			{
				if (Args[3].getKind() == clang::TemplateArgument::ArgKind::Integral) {
					binding = static_cast<unsigned>(Args[3].getAsIntegral().getZExtValue());
				}
			}

			// set is Arg 5
			unsigned set = 0;
			if (Args.size() >= 5)
			{
				if (Args[4].getKind() == clang::TemplateArgument::ArgKind::Integral) {
					set = static_cast<unsigned>(Args[4].getAsIntegral().getZExtValue());
				}
			}

			std::string varName = FD->getNameAsString();
			// Build image layout declaration
			const ImageFormatDescriptor& desc = m_ImageFormats.at(imageFormat.value());
			std::string glsl = "layout(binding=" + std::to_string(binding) +
				"," + desc.imageIdentifier + ") "
				"uniform " + m_TypePrefix.at(desc.scalar) + "image" + m_DimensionSuffix.at(dimension.value()) +
				" " + varName;

			// Replace the entire field declaration (including initializer)
			SourceLocation endLoc = Lexer::getLocForEndOfToken(
				FD->getSourceRange().getEnd(), 1, SM, m_pASTContext->getLangOpts());
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
			classTemplateSpecializationDecl(hasName("ImageBinding"))
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

	clang::SourceManager& sourceManager = m_pASTContext->getSourceManager();
	const clang::LangOptions& languageOptions = m_pASTContext->getLangOpts();

	auto exprText = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(sr), sourceManager, languageOptions);

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

	if (src != dst && ((srcRank > toRank) || src == GLSLDataType::BOOL || dst == GLSLDataType::BOOL))
	{
		// Only warn or edit if GLSL would disallow it
		if (src != GLSLDataType::OTHER && dst != GLSLDataType::OTHER) {
			this->castTo(subExpr, dst);  // Surround the subexpression with explicit cast
		}
	}
	return true;
}

bool KernelRewriter::VisitCallExpr(clang::CallExpr* callExpr)
{
	if (auto* functionCall = callExpr->getDirectCallee()) {

		if (isInNamespace(functionCall, "tc")) {
			// Remove just the namespace qualifier "tc::" if it’s present in the source.
			if (auto* DRE = llvm::dyn_cast<clang::DeclRefExpr>(
				callExpr->getCallee()->IgnoreParenImpCasts())) {
				clang::NestedNameSpecifierLoc Q = DRE->getQualifierLoc();
				if (Q) {
					PendingEdit edit{ Q.getSourceRange(),"" };
					m_PendingEdits.emplace_back(edit);

				}
			}
		}
	}
	return true;
}

bool KernelRewriter::VisitUsingDirectiveDecl(clang::UsingDirectiveDecl* pUsing)
{
	PendingEdit edit{ pUsing->getSourceRange(), "//",true };
	m_PendingEdits.emplace_back(edit);
	return true;
}

void KernelRewriter::focusedDump(const clang::Stmt* S, clang::ASTContext& Ctx) {
	clang::TextNodeDumper D(llvm::errs(), Ctx,
		/*ShowColors=*/true);
	D.Visit(S);  // also works for Decl*, Type, TypeLoc
}



void KernelRewriter::rewriteVecCtorType(const clang::Expr* E) {
	if (!E) return;
	using namespace clang;
	if (auto* TO = dyn_cast<CXXTemporaryObjectExpr>(E->IgnoreParenImpCasts())) {
		if (auto* TSI = TO->getTypeSourceInfo()) {
			QualType QT = TSI->getType();
			if (auto glsl = glslTypeForVecBase(QT)) {
				TypeLoc TL = TSI->getTypeLoc();
				PendingEdit edit{ TL.getSourceRange(),glsl.value() };
				m_PendingEdits.emplace_back(edit);
			}
		}
		if (TO->isListInitialization())
		{
			SourceRange braces = TO->getParenOrBraceRange();
			using namespace clang;
			PendingEdit pe{ braces.getBegin(),"(" };
			m_PendingEdits.emplace_back(pe);

			PendingEdit pe2{ braces.getEnd(),")" };
			m_PendingEdits.emplace_back(pe2);
		}
		return;
	}

	if (auto* FC = dyn_cast<CXXFunctionalCastExpr>(E->IgnoreParenImpCasts())) {
		if (auto* TSI = FC->getTypeInfoAsWritten()) {
			QualType QT = TSI->getType();
			if (auto glsl = glslTypeForVecBase(QT)) {
				TypeLoc TL = TSI->getTypeLoc();
				PendingEdit edit{ TL.getSourceRange(),*glsl };
				m_PendingEdits.emplace_back(edit);
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

	auto* UDL = dyn_cast<clang::UserDefinedLiteral>(E->getArg(1)->IgnoreParenImpCasts());
	if (!UDL) return true;

	auto* ID = UDL->getUDSuffix();
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
