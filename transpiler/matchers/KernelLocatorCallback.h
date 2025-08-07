#pragma once

#include <clang/astmatchers/ASTMatchFinder.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/lex/Lexer.h>

#include "../PendingEdit.h"
#include "../KernelStruct.h"
#include <vector>

class KernelLocatorCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
	KernelLocatorCallback()
	{
	}

	bool isKernelStruct(const clang::CXXRecordDecl* record) const {
		if (record == nullptr) {
			return false;
		}
		for (const auto* A : record->specific_attrs<clang::AnnotateAttr>()) {
			if (A->getAnnotation() == "kernel") {
				return true;
			}
		}
		return false;
	}

	void removeFileLocation(const clang::VarDecl* fileLocVar,
		const clang::SourceManager* sourceManager,
		const clang::LangOptions& languageOptions) {
		using namespace clang;
		if (fileLocVar == nullptr) {
			return;
		}
		SourceRange vardeclRange = fileLocVar->getSourceRange();
		SourceLocation afterSemicolon = Lexer::findLocationAfterToken(
			vardeclRange.getEnd(),                      // start search after end of the VarDecl
			tok::semi,                               // look for a semicolon
			*sourceManager,
			languageOptions,
			true
		);

		SourceRange replacementRange = vardeclRange;
		if (afterSemicolon.isValid()) {
			replacementRange = SourceRange{ vardeclRange.getBegin(),afterSemicolon };
		}

		PendingEdit removeFileLoc{ replacementRange,"" };
		m_PendingEdits.emplace_back(removeFileLoc);
	}

	std::optional<unsigned> extractUInt(const clang::Expr* E, clang::ASTContext& Ctx) {
		using namespace clang;
		if (!E) return std::nullopt;
		E = E->IgnoreImplicit(); // strip casts, etc.

		// Direct integer literal
		if (const auto* IL = dyn_cast<IntegerLiteral>(E)) {
			return static_cast<unsigned>(IL->getValue().getZExtValue());
		}

		// Try constant evaluation (e.g., more complex constexpr expressions)
		Expr::EvalResult Result;
		if (E->EvaluateAsRValue(Result, Ctx)) {
			if (Result.Val.hasValue()) {
				const llvm::APSInt AI = Result.Val.getInt();
				return static_cast<unsigned>(AI.getZExtValue());
			}
		}

		return std::nullopt;
	}

	void convertLocalSize(const clang::FieldDecl* pField, clang::ASTContext* Ctx)
	{
		using namespace clang;
		if (!pField->hasInClassInitializer())
			return;

		const Expr* Init = pField->getInClassInitializer()->IgnoreImplicit();

		// Expect a constructor expression: sf::uvec3{18,18,1} or sf::uvec3(18,18,1)
		const auto* CCE = dyn_cast<CXXConstructExpr>(Init);
		if (!CCE)
			return;

		if (CCE->getNumArgs() != 3)
			return;

		auto xOpt = extractUInt(CCE->getArg(0), *Ctx);
		auto yOpt = extractUInt(CCE->getArg(1), *Ctx);
		auto zOpt = extractUInt(CCE->getArg(2), *Ctx);
		if (!xOpt || !yOpt || !zOpt) {
			llvm::errs() << "Failed to evaluate one of the local_size components\n";
			return;
		}

		unsigned x = *xOpt;
		unsigned y = *yOpt;
		unsigned z = *zOpt;

		llvm::outs() << "Extracted local_size: " << x << ", " << y << ", " << z << "\n";

		// Build replacement GLSL layout line
		std::string replacement =
			"layout (local_size_x = " + std::to_string(x) +
			", local_size_y = " + std::to_string(y) +
			", local_size_z = " + std::to_string(z) +
			") in;";

		// Replace the entire field declaration (including initializer)
		SourceManager& SM = Ctx->getSourceManager();
		SourceLocation endLoc = Lexer::getLocForEndOfToken(
			pField->getSourceRange().getEnd(), 0, SM, Ctx->getLangOpts());
		SourceRange fullRange(pField->getSourceRange().getBegin(), endLoc);
		m_PendingEdits.emplace_back(PendingEdit{ fullRange,replacement });
	}

	void run(const clang::ast_matchers::MatchFinder::MatchResult& Result) override {
		using namespace clang;
		const clang::CXXRecordDecl* pKernel = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("kernelStruct");
		if (isKernelStruct(pKernel))
		{
			const clang::VarDecl* fileLoc = Result.Nodes.getNodeAs<clang::VarDecl>("fileLocation");
			const clang::Expr* init = fileLoc->getInit();
			std::string exportLocation;
			if (const auto* SL = llvm::dyn_cast<StringLiteral>(init->IgnoreImplicit())) {
				exportLocation = SL->getString().str(); 
			}
			KernelStruct ks{ pKernel, exportLocation };
			m_FoundKernels.emplace_back(ks);
			removeFileLocation(fileLoc, Result.SourceManager, Result.Context->getLangOpts());

			const clang::FieldDecl* pField = Result.Nodes.getNodeAs<clang::FieldDecl>("localSizeVar");
			convertLocalSize(pField, Result.Context);

			for (const auto* method : pKernel->methods())
			{
				if (method->getNameAsString().starts_with("_"))
				{
					PendingEdit edit{ method->getSourceRange(), "" };
					m_PendingEdits.emplace_back(edit);
				}
			}
		}
	}

	std::vector<KernelStruct>& getKernels()
	{
		return m_FoundKernels;
	}

	std::vector<PendingEdit>& getPendingEdits()
	{
		return m_PendingEdits;
	}
private:
	std::vector<PendingEdit> m_PendingEdits;
	std::vector<KernelStruct> m_FoundKernels;
};