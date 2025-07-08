#pragma once

#include <GL/glew.h>

#include <string>
#include <array>


template<typename T, unsigned Size, unsigned Set, unsigned Binding>
class ShaderBuffer
{
public:
	ShaderBuffer(); 
	~ShaderBuffer();
	
	const T operator[](unsigned idx) const
	{
		return m_Data[idx];
	}

	T& operator[](unsigned idx) {
		return m_Data[idx];
	}

	void bindAsCompute(unsigned m_BindingID);
	
	unsigned size() const;

	static const unsigned SET = Set;
	static const unsigned BINDING = Binding;
private:
	std::array<T,Size> m_Data;

	// To release
	unsigned m_SSBO_ID;
};

template <typename T, unsigned Size, unsigned Set, unsigned Binding>
ShaderBuffer<T, Size, Set, Binding>::ShaderBuffer()
{
	glGenBuffers(1, &m_SSBO_ID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_ID);

	// Allocate memory for the SSBO and upload the data
	int size = sizeof(T);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_Data.size()*sizeof(T), m_Data.data(), GL_STATIC_DRAW);

	// Bind the SSBO to a specific binding point (e.g., binding point 0)
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_SSBO_ID);

	// Unbind the buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}




template <typename T, unsigned Size, unsigned Set, unsigned Binding>
ShaderBuffer<T,Size,Set,Binding>::~ShaderBuffer()
{
	if (m_SSBO_ID != 0)
	{
		glDeleteBuffers(1, &m_SSBO_ID);
		m_SSBO_ID = 0;
	}
}

template <typename T, unsigned Size, unsigned Set, unsigned Binding>
void ShaderBuffer<T,Size,Set,Binding>::bindAsCompute(unsigned bindingID)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingID, m_SSBO_ID);
}

template <typename T,unsigned Size, unsigned Set, unsigned Binding>
unsigned ShaderBuffer<T,Size,Set,Binding>::size() const
{
	return m_Data.size();
}