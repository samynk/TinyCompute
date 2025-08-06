#pragma once

#include "computebackend.hpp"
#include "ComputeShader.h"

class GPUBackend : public sf::ComputeBackend<GPUBackend>
{
public:
	GPUBackend() : sf::ComputeBackend<GPUBackend>{ sf::BackendType::OpenGL }
	{

	}

	template<typename BufferType>
	void uploadBuffer(std::span<BufferType> data, unsigned Set, unsigned BindId)
	{
		// no op
	}

	template<typename BufferType>
	void downloadBuffer(std::span<BufferType> data, unsigned Set, unsigned BindId)
	{
		// no op
	}

	template<KernelEntry K>
	void executeImpl(K& kernel, size_t totalWork)
	{
		// check if kernel was compiled.
		checkKernel(kernel);
	}
private:
	template<KernelEntry K>
	void checkKernel(K& kernel)
	{
		if (m_CompiledPrograms.find(kernel.fileLocation) == m_CompiledPrograms.end())
		{
			std::string fileLoc = std::string(kernel.fileLocation) + ".glsl";
			ComputeShader shader{ fileLoc};
			shader.compile();
			std::string key = kernel.fileLocation;
			m_CompiledPrograms.insert_or_assign(key, std::move(shader));
		}
	}

	static inline std::unordered_map<std::string, ComputeShader> m_CompiledPrograms;
};