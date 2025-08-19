#pragma once

#include "computebackend.hpp"
#include "kernel_intrinsics.hpp"
#include "ComputeShader.h"

class GPUBackend : public tc::ComputeBackend<GPUBackend>
{
public:
	GPUBackend() : tc::ComputeBackend<GPUBackend>{ tc::BackendType::OpenGL }
	{

	}

	template<typename BufferType>
	void uploadBufferImpl(tc::BufferResource<BufferType>& buffer)
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
	void downloadBufferImpl(tc::BufferResource<BufferType>& buffer)
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
	void bindBufferImpl(const tc::BufferBinding<T, Binding, Set>& buffer)
	{
		// This assumes you've already uploaded the data and stored the buffer ID
		unsigned int bufferID = buffer.getBufferData()->getSSBO_ID();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Binding, bufferID);
		// Optional: You could log it
		std::cout << "Bound buffer to binding=" << Binding << ", set=" << Set << "\n";
	}

	template<tc::PixelType P>
	void uploadImageImpl(tc::GPUFormat gpuFormat, tc::BufferResource<P, tc::Dim::D2>& buffer)
	{
		static_assert(P::NumChannels >= 1 && P::NumChannels <= 4,
			"Pixel NumChannels must be 1..4");
		unsigned int bufferID;
		if ( (bufferID = buffer.getSSBO_ID()) == std::numeric_limits<unsigned int>::max())
		{
			glGenTextures(1, &bufferID);
			buffer.setSSBO_ID(bufferID);
			std::cout << "Generated buffer with id " << bufferID << "\n";	
		}

		if (bufferID == 0) {
			throw std::runtime_error("Failed to generate texture.");
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bufferID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Determine the format based on the number of channels
		GLenum format = GL_RGB;
		switch (P::NumChannels) {
		case 1:
			format = GL_RED;
			break;
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
		default:
			throw std::runtime_error("Unsupported number of channels in texture image.");
		}

		// Upload texture data
		tc::uvec2 dim = buffer.getDimension();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, dim.x, dim.y, 0, format, GL_UNSIGNED_BYTE, buffer.data());

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			throw std::runtime_error("OpenGL Error in OpenGLBackend::uploadImageImpl : " + std::to_string(error));
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	template<unsigned Location>
	void bindUniformImpl(const tc::Uniform<float, Location>& uniform)
	{

		glUniform1f(Location, uniform.get());
	}

	// Specialization for int
	template<unsigned Location>
	void bindUniformImpl(const tc::Uniform<int, Location>& uniform)
	{
		std::cout << "Setting uniform on location " << Location << ":" << uniform.get() << "\n";
		glUniform1i(Location, uniform.get());
	}

	// Specialization for unsigned int
	template<unsigned Location>
	void bindUniformImpl(const tc::Uniform<unsigned int, Location>& uniform)
	{
		std::cout << "Setting uniform on location " << Location << ":" << uniform.get() << "\n";
		glUniform1ui(Location, uniform.get());
	}

	template<KernelEntry K>
	void executeImpl(K& kernel, const tc::uvec3 globalWorkSize)
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
	void useKernelImpl(K& kernel)
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