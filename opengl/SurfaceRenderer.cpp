#include "SurfaceRenderer.hpp"
#include "OpenGLBackend.hpp"

#include <vector>
#include <stdexcept>
#include <fstream>
#include <sstream>

SurfaceRenderer::SurfaceRenderer(GLuint w, GLuint h)
	:
	m_FullScreenImage{ tc::ivec2{w,h} },
	m_ProgramID{ 0 },
	m_VertexArrayObject{ 0 },
	m_VertexBufferObject{ 0 },
	m_Width{ w },
	m_Height{ h }
{
}

SurfaceRenderer::~SurfaceRenderer()
{
	if (m_ProgramID != 0) {
		glDeleteProgram(m_ProgramID);
		m_ProgramID = 0;
	}
	if (m_VertexArrayObject != 0)
	{
		glDeleteVertexArrays(1, &m_VertexArrayObject);
		m_VertexArrayObject = 0;
	}
	if (m_VertexBufferObject != 0)
	{
		glDeleteBuffers(1, &m_VertexBufferObject);
	}
}

void SurfaceRenderer::init()
{
	createShaderProgram();
	
	updateTexture();
	setupQuad();
}

void SurfaceRenderer::updateTexture()
{
	if (m_FullScreenImage.isOnCPU())
	{
		tc::gpu::GPUBackend gpu;
		gpu.uploadImage<tc::InternalFormat::RGBA8>(m_FullScreenImage);
	}
}

void SurfaceRenderer::createShaderProgram()
{
	GLuint vertexShaderID = compileShader(gVertexShader, GL_VERTEX_SHADER);
	GLuint fragmentShaderID = compileShader(gFragmentShader, GL_FRAGMENT_SHADER);

	// Link shaders into a program
	m_ProgramID = glCreateProgram();
	glAttachShader(m_ProgramID, vertexShaderID);
	glAttachShader(m_ProgramID, fragmentShaderID);
	glLinkProgram(m_ProgramID);

	// Check for linking errors
	GLint success;
	glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &success);
	if (!success) {
		std::vector<char> infoLog(1024);
		glGetProgramInfoLog(m_ProgramID, 1024, nullptr, infoLog.data());
		throw std::runtime_error("ERROR::PROGRAM::LINKING_FAILED\n" + std::string(infoLog.begin(), infoLog.end()));
	}
	m_screenTextureLoc = glGetUniformLocation(m_ProgramID, "screenTexture");
	// Shaders can be deleted after linking
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
}

void SurfaceRenderer::setupQuad()
{
	glGenVertexArrays(1, &m_VertexArrayObject);
	glGenBuffers(1, &m_VertexBufferObject);

	glBindVertexArray(m_VertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Texture coordinate attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SurfaceRenderer::drawQuadWithTexture()
{
	glUseProgram(m_ProgramID);
	glUniform1i(m_screenTextureLoc, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_FullScreenImage.getSSBO_ID());

	// Draw the quad
	glBindVertexArray(m_VertexArrayObject);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

GLuint SurfaceRenderer::compileShader(const std::string_view& shaderSource, GLenum shaderType) const
{
	GLuint shaderID = glCreateShader(shaderType);
	const char* computeShaderSource = shaderSource.data();
	GLint length = shaderSource.size();

	glShaderSource(shaderID, 1, &computeShaderSource, &length);
	glCompileShader(shaderID);

	// Check for compilation errors
	GLint success;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if (!success) {
		// Retrieve and log the error message
		std::vector<char> infoLog(1024);
		glGetShaderInfoLog(shaderID, static_cast<GLsizei>(infoLog.size()), nullptr, infoLog.data());
		std::string errorMessage(infoLog.begin(), infoLog.end());
		throw std::runtime_error("ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" + errorMessage);
	}

	return shaderID;
}
