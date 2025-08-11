#pragma once

#include "computebackend.hpp"
#include "kernel_intrinsics.hpp"
#include "ComputeShader.h"

class GPUBackend : public sf::ComputeBackend<GPUBackend>
{
public:
	GPUBackend() : sf::ComputeBackend<GPUBackend>{ sf::BackendType::OpenGL }
	{

	}

	template<typename BufferType>
	void uploadBuffer(sf::BufferResource<BufferType>& buffer)
	{
		if (buffer.getSSBO_ID() == std::numeric_limits<unsigned int>::max())
		{
			unsigned int bufferID;
			glGenBuffers(1, &bufferID);
			buffer.setSSBO_ID(bufferID);
			std::cout << "Generated buffer with id " << bufferID << "\n";
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.getSSBO_ID());

		// Allocate memory for the SSBO and upload the data
		glBufferData(
			GL_SHADER_STORAGE_BUFFER, 
			buffer.size() * sizeof(BufferType), 
			buffer.data(), 
			GL_STATIC_DRAW
		);

		// Bind the SSBO to a specific binding point (e.g., binding point 0)
		// glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_SSBO_ID);

		// Unbind the buffer
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	template<typename BufferType>
	void downloadBuffer(sf::BufferResource<BufferType>& buffer)
	{
		if (buffer.getSSBO_ID() == std::numeric_limits<unsigned int>::max())
		{
			std::cerr << "Error: Trying to download from an uninitialized buffer.\n";
			return;
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.getSSBO_ID());

		// Map the buffer to read from GPU
		void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		if (!ptr)
		{
			std::cerr << "Error: Failed to map SSBO for reading.\n";
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			return;
		}

		// Copy the data into the CPU-side buffer
		std::memcpy(buffer.data(), ptr, buffer.size() * sizeof(BufferType));

		// Unmap and unbind
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	template<typename T, unsigned Binding, unsigned Set>
	void bindBuffer(const sf::BindingPoint<T, Binding, Set>& buffer)
	{
		// This assumes you've already uploaded the data and stored the buffer ID
		unsigned int bufferID = buffer.getBufferData()->getSSBO_ID();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Binding, bufferID);
		// Optional: You could log it
		std::cout << "Bound buffer to binding=" << Binding << ", set=" << Set << "\n";
	}

	template<unsigned Location>
	void bindUniform(const sf::Uniform<float, Location>& uniform)
	{

		glUniform1f(Location, uniform.get());
	}

	// Specialization for int
	template<unsigned Location>
	void bindUniform(const sf::Uniform<int, Location>& uniform)
	{
		std::cout << "Setting uniform on location " << Location << ":" << uniform.get() << "\n";
		glUniform1i(Location, uniform.get());
	}

	// Specialization for unsigned int
	template<unsigned Location>
	void bindUniform(const sf::Uniform<unsigned int, Location>& uniform)
	{
		std::cout << "Setting uniform on location " << Location << ":" << uniform.get() << "\n";
		glUniform1ui(Location, uniform.get());
	}

	template<KernelEntry K>
	void executeImpl(K& kernel, const sf::uvec3 globalWorkSize)
	{
		GLuint workGroupCountX = globalWorkSize.x / kernel.local_size.x;
		GLuint workGroupCountY = globalWorkSize.y / kernel.local_size.y;
		GLuint workGroupCountZ = globalWorkSize.z / kernel.local_size.z;
		// Dispatch compute shader
		glDispatchCompute(workGroupCountX, workGroupCountY, workGroupCountZ);
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			throw std::runtime_error("OpenGL Error in ComputeShader::dispatchCompute(): " + std::to_string(error));
		}
	}

	template<KernelEntry K>
	void useKernel(K& kernel)
	{
		// check if kernel was compiled.
		checkKernel(kernel);
		ComputeShader& shader = m_CompiledPrograms[kernel.fileLocation];
		shader.use();
	}
private:
	template<KernelEntry K>
	void checkKernel(K& kernel)
	{
		if (m_CompiledPrograms.find(kernel.fileLocation) == m_CompiledPrograms.end())
		{
			std::string fileLoc = std::string(kernel.fileLocation) + ".comp";
			ComputeShader shader{ fileLoc};
			shader.compile();
			std::string key = kernel.fileLocation;
			m_CompiledPrograms.insert_or_assign(key, std::move(shader));
		}
	}

	static inline std::unordered_map<std::string, ComputeShader> m_CompiledPrograms;
};