#pragma once

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
class BindingPointCallback : public MatchFinder::MatchCallback {
public:
	BindingPointCallback(std::vector<PendingEdit>& edits) :m_PendingEdits(edits) {}

	std::optional<std::string> glslTypeForElement(QualType elemType, ASTContext& Ctx, bool& promotedFrom8) {
		elemType = elemType.getCanonicalType();
		promotedFrom8 = false;
		if (elemType->isUnsignedIntegerType()) {
			// detect uint8_t by spelling if needed; on MSVC it may be \"unsigned char\"
			auto name = elemType.getAsString();
			if (name == "unsigned char" || name == "uint8_t") {
				promotedFrom8 = true;
				return std::string("uint"); // promote to uint
			}
			return std::string("uint");
		}
		if (elemType->isIntegerType()) return std::string("int");
		if (elemType->isFloatingType()) return std::string("float");
		if (elemType->isBooleanType()) return std::string("bool");
		return std::nullopt;
	}

	void handleBindingPoint(const FieldDecl* FD, const MatchFinder::MatchResult& Result) {
		ASTContext& Ctx = *Result.Context;
		const SourceManager& SM = Ctx.getSourceManager();

		const QualType QT = FD->getType();
		const TemplateSpecializationType* TST = QT->getAs<TemplateSpecializationType>();
		if (!TST) return;

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

				bool promotedFrom8 = false;
				auto glslElemTypeOpt = glslTypeForElement(elemType, Ctx, promotedFrom8);
				std::string glslElemType = *glslElemTypeOpt;

				std::string varName = FD->getNameAsString();
				// Build GLSL buffer declaration (SSBO style)
				std::string glsl = "layout(set = " + std::to_string(set) +
					", binding = " + std::to_string(binding) + ") buffer " + varName + " {"
					+ glslElemType + " " + varName + "[]; }; ";

				llvm::errs() << "Replacing binding with :\n" << glsl << "\n.";

				// Replace the entire field declaration (including initializer)
				SourceLocation endLoc = Lexer::getLocForEndOfToken(
					FD->getSourceRange().getEnd(), 0, SM, Ctx.getLangOpts());
				SourceRange fullRange(FD->getSourceRange().getBegin(), endLoc);

				PendingEdit edit{ fullRange, glsl };
				m_PendingEdits.emplace_back(edit);
			}
		}

		
	}

	void handleUniform(const FieldDecl* FD, const MatchFinder::MatchResult& Result) {
		ASTContext& Ctx = *Result.Context;
		const SourceManager& SM = Ctx.getSourceManager();

		const QualType QT = FD->getType();
		const TemplateSpecializationType* TST = QT->getAs<TemplateSpecializationType>();
		if (!TST) return;

		QualType elemType;
		unsigned binding = 0, set = 0;

		if (const auto* CTSDecl = dyn_cast<ClassTemplateSpecializationDecl>(
			TST->getAsRecordDecl())) {

			const auto& Args = CTSDecl->getTemplateArgs();
			if (Args.size() >= 2) {
				// Elem type is Arg 0
				QualType elemType = Args[0].getAsType();

				// binding is Arg 1
				unsigned location = 0;
				if (Args[1].getKind() == clang::TemplateArgument::ArgKind::Integral) {
					binding = static_cast<unsigned>(Args[1].getAsIntegral().getZExtValue());
				}

				bool promotedFrom8 = false;
				auto glslElemTypeOpt = glslTypeForElement(elemType, Ctx, promotedFrom8);
				std::string glslElemType = *glslElemTypeOpt;

				std::string varName = FD->getNameAsString();
				std::string name = FD->getNameAsString();
				std::string uniformDecl = "layout(location = " + std::to_string(location) +
					") uniform " + glslElemType + " " + name + ";\n";

				llvm::errs() << "Replacing location with :\n" << uniformDecl << "\n.";

				// Replace the entire field declaration (including initializer)
				SourceLocation endLoc = Lexer::getLocForEndOfToken(
					FD->getSourceRange().getEnd(), 0, SM, Ctx.getLangOpts());
				SourceRange fullRange(FD->getSourceRange().getBegin(), endLoc);

				// Do the replacement
				PendingEdit edit{ fullRange, uniformDecl };
				m_PendingEdits.emplace_back(edit);
			}
		}
	}

	void run(const MatchFinder::MatchResult& Result) override {
		if (const FieldDecl* BF = Result.Nodes.getNodeAs<FieldDecl>("bindingField")) {
			handleBindingPoint(BF, Result);
		}
		if (const FieldDecl* UF = Result.Nodes.getNodeAs<FieldDecl>("uniformField")) {
			handleUniform(UF, Result);
		}
	}

private:
	std::vector<PendingEdit>& m_PendingEdits;
};