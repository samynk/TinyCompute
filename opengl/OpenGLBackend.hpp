#pragma once

#include "GL/glew.h"

#include "ComputeShader.hpp"

#include "computebackend.hpp"
#include "kernel_intrinsics.hpp"
#include "images/ImageFormat.hpp"

#include <unordered_map>
#include <cstring> // memcpy

namespace tc::gpu {

	class GPUBackend : public tc::ComputeBackend<GPUBackend>
	{
	public:
		GPUBackend() : tc::ComputeBackend<GPUBackend>{ tc::BackendType::OpenGL }
		{

		}

		template<typename BufferType>
		void uploadBufferImpl(tc::BufferResource<BufferType>& buffer)
		{
			if (buffer.getSSBO_ID() == 0)
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
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}

		template<typename BufferType>
		void downloadBufferImpl(tc::BufferResource<BufferType>& buffer)
		{
			if (buffer.getSSBO_ID() == 0)
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
			unsigned int bufferID = buffer.getBufferData()->getSSBO_ID();
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Binding, bufferID);
		}

		template<tc::GPUFormat G>
		struct OpenGLFormatTraits;

		template<>
		struct OpenGLFormatTraits<tc::GPUFormat::RGBA32F> {
			static constexpr GLuint internalType = GL_RGBA32F;
			static constexpr uint8_t NumChannels = 4;
		};

		template<>
		struct OpenGLFormatTraits<tc::GPUFormat::RGBA8> {
			static constexpr GLuint internalType = GL_RGBA8;
			static constexpr uint8_t NumChannels = 4;
		};

		template<>
		struct OpenGLFormatTraits<tc::GPUFormat::R8UI> {
			static constexpr GLuint internalType = GL_R8UI;
			static constexpr uint8_t NumChannels = 1;
		};

		template<tc::cpu::PixelType> struct OpenGLExternalTraits;

		template<> struct OpenGLExternalTraits<tc::cpu::R8UI> {
			static constexpr GLenum format = GL_RED_INTEGER;
			static constexpr GLenum type = GL_UNSIGNED_BYTE;
			static constexpr int    channels = 1;
		};

		//template<> struct OpenGLExternalTraits<tc::R8> {
		//	static constexpr GLenum format = GL_RED;              // UNorm path
		//	static constexpr GLenum type = GL_UNSIGNED_BYTE;
		//	static constexpr int    channels = 1;
		//	static constexpr int    bytesPerPixel = 1;
		//};

		template<> struct OpenGLExternalTraits<tc::cpu::RGBA8UI> {
			static constexpr GLenum format = GL_RGBA;
			static constexpr GLenum type = GL_UNSIGNED_BYTE;
			static constexpr int    channels = 4;
			static constexpr int    bytesPerPixel = 4;
		};

		template<> struct OpenGLExternalTraits<tc::cpu::RGBA8> {
			static constexpr GLenum format = GL_RGBA;
			static constexpr GLenum type = GL_FLOAT;
			static constexpr int    channels = 4;
			static constexpr int    bytesPerPixel = 16;
		};

		template<tc::GPUFormat G, tc::cpu::PixelType P>
		void uploadImageImpl(tc::BufferResource<P, tc::Dim::D2>& buffer)
		{
			static_assert(P::NumChannels >= 1 && P::NumChannels <= 4,
				"Pixel NumChannels must be 1..4");
			unsigned int bufferID;
			if ((bufferID = buffer.getSSBO_ID()) == 0)
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
			// Upload texture data
			tc::ivec2 dim = buffer.getDimension();
			glTexImage2D(GL_TEXTURE_2D, 0,
				OpenGLFormatTraits<G>::internalType,
				dim.x, dim.y, 0,
				OpenGLExternalTraits<P>::format, OpenGLExternalTraits<P>::type,
				buffer.data()
			);

			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
			{
				throw std::runtime_error("OpenGL Error in OpenGLBackend::uploadImageImpl : " + std::to_string(error));
			}
			glBindTexture(GL_TEXTURE_2D, 0);
		}


		template<tc::GPUFormat G, tc::Dim D, tc::cpu::PixelType P, unsigned B, unsigned S>
		void bindImageImpl(const tc::ImageBinding<G, D, P, B, S>& image)
		{
			unsigned int imageID = image.getBufferData()->getSSBO_ID();
			unsigned int internalType = OpenGLFormatTraits<G>::internalType;
			uint8_t binding = B;
			glBindImageTexture(binding, imageID, 0, GL_FALSE, 0, GL_READ_WRITE, internalType);
			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
			{
				throw std::runtime_error("OpenGL Error in GLImage::bind(): " + std::to_string(error));
			}
		}

		template<int Location, typename T>
		void bindUniformImpl(const tc::Uniform<T, Location>& uniform)
		{
			if constexpr (std::is_same_v<T, float>)
				glUniform1f(Location, uniform.get());
			else if constexpr (std::is_same_v<T, int>)
				glUniform1i(Location, uniform.get());
			else if constexpr (std::is_same_v <T, unsigned int>)
				glUniform1ui(Location, uniform.get());
			else
				static_assert(false, "Unsupported uniform type for bindUniformImpl.");
		}

		template<KernelEntry K>
		void executeImpl(K& kernel, const tc::uvec3 globalWorkSize)
		{

			GLuint workGroupCountX = ceil_div(globalWorkSize.x, kernel.local_size.x);
			GLuint workGroupCountY = ceil_div(globalWorkSize.y, kernel.local_size.y);
			GLuint workGroupCountZ = ceil_div(globalWorkSize.z, kernel.local_size.z);
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
				ComputeShader shader{ fileLoc };
				shader.compile();
				std::string key = kernel.fileLocation;
				m_CompiledPrograms.insert_or_assign(key, std::move(shader));
			}
		}

		static inline std::unordered_map<std::string, ComputeShader> m_CompiledPrograms;
	};
}