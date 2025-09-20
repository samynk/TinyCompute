#pragma once
#include <clang/AST/CXXInheritance.h>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>

class KernelStruct {
public:
	KernelStruct(const clang::CXXRecordDecl* kernelRecord, std::string fileLocation)
		:m_Kernel{ kernelRecord }, m_FileLoc{ fileLocation }
	{
		/*
		auto unsignedMatcher = varDecl(
			anyOf(
				varDecl(hasType(isUnsignedInteger())).bind("unsignedVar"),
				parmVarDecl(hasType(isUnsignedInteger())).bind("unsignedVar"),
				fieldDecl(hasType(isUnsignedInteger())).bind("unsignedVar"),
				functionDecl(returns(isUnsignedInteger())).bind("unsignedReturn")
			)
		).bind("unsigned"); */
	}

	const clang::CXXRecordDecl* getKernelRecordDecl() const {
		return m_Kernel;
	}

	void rewrite(clang::ASTContext* Ctx)
	{



	}

	void exportComputeShader(clang::Rewriter& rewriter, std::string outputDir)
	{
		llvm::errs() << "Inside export compute shader\n";
		clang::SourceManager& sourceManager = rewriter.getSourceMgr();
		const clang::LangOptions& languageOptions = rewriter.getLangOpts();
		
		SourceRange range = m_Kernel->getBraceRange();
		SourceLocation contentStart = clang::Lexer::getLocForEndOfToken(
			sourceManager.getExpansionLoc(range.getBegin()), 0, sourceManager, languageOptions);

		SourceLocation contentEnd = sourceManager.getExpansionLoc(range.getEnd().getLocWithOffset(-1));

		std::string computeShader = rewriter.getRewrittenText(SourceRange{ contentStart,contentEnd });
		std::filesystem::path path = outputDir;
		std::filesystem::path fileLocPath = m_FileLoc;
		std::filesystem::path glslFile = fileLocPath.replace_extension("comp");
		path /= glslFile;
		
		std::filesystem::create_directories(path.parent_path());


		std::string shader = "#version 430\n" + computeShader;
		std::string normalized;
		normalized = normalizeLineEndings(shader);
		std::ofstream ofs(path);
		ofs << normalized;
		ofs.close();

		std::string log;
		auto errorCode = validateShader(path.string(), log);
		if (errorCode != 0) {
			llvm::errs() << "Execution of glslangValidator failed";
		}
	}


private:
	bool validateShader(
		const std::string& shaderFile,
		std::string& outLog) {
		/*
		std::vector<llvm::StringRef> args = {
			shaderFile
		};

		llvm::Optional<llvm::sys::Program> P =
			llvm::sys::Program::FindProgramByName("glslangValidator.exe", args);

		if (!P) {
			llvm::errs() << "Error: Could not find glslangValidator.exe \n";
			return false;
		}

		int result = P->ExecuteAndWait(P, args, llvm::None, llvm::None, 0, nullptr, nullptr);

		if (result != 0) {
			llvm::errs() << "Error: Program " << program << " exited with code " << result << "\n";
			return 1;
		}
		//formatErrors(outFile, shaderFile);
		return exitCode == 0;  // zero means “valid”
		*/
		return true;
	}

	void formatErrors(const std::string& path, const std::string& shaderFile) {
		std::ifstream inFile(path, std::ios::in);
		if (inFile.is_open())
		{
			std::string line;
			std::regex errorPattern(R"(^ERROR:\s(\d+):(\d+):\s*(.*)$)");
			while (getline(inFile, line))
			{
				// llvm::errs() << "trying to match :" << line << "\n";
				if (line.empty()) continue;  // Skip blank lines

				std::smatch match;
				if (std::regex_match(line, match, errorPattern)) {

					std::string lineNumber = match[2].str();
					std::string message = match[3].str();

					std::ostringstream formatted;
					formatted << shaderFile
						<< "(" << lineNumber << "): error: "
						<< message << "\n";
					llvm::errs() << formatted.str();
				}
			}

		}
	}

	std::string normalizeLineEndings(const std::string& input, const std::string& target = "\n") {
		std::string output;
		output.reserve(input.size());

		for (size_t i = 0; i < input.size(); ++i) {
			if (input[i] == '\r') {
				if (i + 1 < input.size() && input[i + 1] == '\n') {
					// Windows-style line ending: \r\n
					++i;
				}
				output += target;
			}
			else if (input[i] == '\n') {
				output += target;
			}
			else {
				output += input[i];
			}
		}

		return output;
	}


	const clang::CXXRecordDecl* m_Kernel;
	const std::string m_FileLoc;
};