#pragma once
#include <string>
#include "GL/glew.h"
#include "glm/glm.hpp"

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
	GLuint m_LocalSizeX = 16;
	GLuint m_LocalSizeY = 16;
	GLuint m_LocalSizeZ = 1;
};
