




#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include "clang/Frontend/TextDiagnostic.h"
#include <llvm/Support/CommandLine.h>
#include <iostream>
#include <vector>

#include "TranspileAction.h"

using namespace clang;
using namespace clang::tooling;

static llvm::cl::OptionCategory MyToolCategory("kernel-transpiler options");

static llvm::cl::opt<std::string> OutputPath(
	"o", llvm::cl::desc("Output file"), llvm::cl::value_desc("filename"),
	llvm::cl::Required, llvm::cl::cat(MyToolCategory));


class MyDiagnosticConsumer : public clang::TextDiagnosticPrinter {
public:

	MyDiagnosticConsumer(raw_ostream& os, DiagnosticOptions& DiagOpts,
		bool OwnsOutputStream = false) 
		:TextDiagnosticPrinter(os,DiagOpts,OwnsOutputStream)
	{

	}

	void HandleDiagnostic(clang::DiagnosticsEngine::Level DiagLevel,
		const clang::Diagnostic& Info) override {

		clang::SmallString<100> Message;
		Info.FormatDiagnostic(Message);

		const clang::SourceManager& SM = Info.getSourceManager();
		clang::SourceLocation Loc = Info.getLocation();
		clang::PresumedLoc PLoc = SM.getPresumedLoc(Loc);

		if (!PLoc.isValid()) {
			llvm::errs() << Message << "\n";
			return;
		}

		
		const char* levelStr =
			(DiagLevel == clang::DiagnosticsEngine::Error || DiagLevel == clang::DiagnosticsEngine::Fatal)
			? "error"
			: (DiagLevel == clang::DiagnosticsEngine::Warning ? "warning" : "note");

		llvm::errs() << PLoc.getFilename() << "(" << PLoc.getLine() << "," << PLoc.getColumn()
			<< "): " << levelStr << ": " << Message << "\n";
	}
};

int main(int argc, const char** argv) {

	auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
	if (!ExpectedParser) {
		llvm::errs() << ExpectedParser.takeError();
		return 1;
	}

	auto diagnosticOptions = std::make_unique<clang::DiagnosticOptions>();
	diagnosticOptions->setFormat(TextDiagnosticFormat::MSVC);
	diagnosticOptions->ShowColors = false;
	diagnosticOptions->ShowLocation = true;
	diagnosticOptions->ShowOptionNames = true;

	clang::TextDiagnosticPrinter* printer =
		new clang::TextDiagnosticPrinter(llvm::errs(), *diagnosticOptions);
	printer->setPrefix("kernelValidation");
	MyDiagnosticConsumer consumer{ llvm::errs(),*diagnosticOptions };
	
	std::string outPath = OutputPath.getValue();
	llvm::outs() << "Writing shaders to: " << outPath << "\n";

	ClangTool Tool(
		ExpectedParser->getCompilations(),
		ExpectedParser->getSourcePathList()
	);

	Tool.setDiagnosticConsumer(&consumer);
	return Tool.run(newFrontendActionFactory<TranspileAction>().get());

}
