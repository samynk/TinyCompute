#pragma once
#include <string>
#include <array>
#include "GL/glew.h"



class ComputeShader
{
public:
	ComputeShader() {}
	ComputeShader(const std::string& fileLocation);
	~ComputeShader();
	// Delete copy constructor and copy assignment to prevent copying
	ComputeShader(const ComputeShader&) = delete;
	ComputeShader& operator=(const ComputeShader&) = delete;

	// Allow move semantics
	ComputeShader(ComputeShader&& other) noexcept;
	ComputeShader& operator=(ComputeShader&& other) noexcept;
	
	void compile();
	void use() const;
private:
	std::string m_FileLocation;
	bool m_SourceValid{ false };
	std::string m_ShaderContents;
	GLint m_SourceLength{ 0 };
	// opengl specific --> resource that should be released
	GLuint m_ShaderID = 0;
	GLuint m_ComputeProgramID = 0;
	// reasonable default for workgroup sizes.
	std::array<GLint, 3>  m_LocalSize = { 16,16,1 };
};
