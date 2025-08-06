#pragma once

#include <execution>
#include <algorithm>
#include <ranges>
#include <span>
#include <numeric>
#include <concepts>

#include "kernel_intrinsics.hpp"

#include <unordered_map>


namespace sf
{
	enum class BackendType {
		CPU, OpenGL
	};

	enum class AccessType {
		READ, WRITE, READWRITE
	};

	

	template <typename Derived>
	class ComputeBackend
	{
	public:

		ComputeBackend(BackendType type) :m_Backend{ type }
		{

		}

		virtual BackendType GetType() {
			return m_Backend;
		}

		template<typename BufferType>
		void uploadBuffer(std::span<BufferType> data, unsigned Set, unsigned BindId)
		{
			static_cast<Derived*>(this)->uploadImpl(data, Set, BindId);
		}

		template<typename BufferType>
		void downloadBuffer(std::span<BufferType> data, unsigned Set, unsigned BindId)
		{
			static_cast<Derived*>(this)->downloadImpl(data, Set, BindId);
		}

		template<KernelEntry K>
		void execute(K& k, size_t totalWork)
		{
			static_assert(HasLocalSize<K>,
				"Kernel must have a 'sf::uvec3 local_size' member.");
			static_cast<Derived*>(this)->executeImpl(k, totalWork);
		}
	private:
		BackendType m_Backend;
	};

	class CPUBackend : public ComputeBackend<CPUBackend>
	{
	public:
		CPUBackend() : ComputeBackend{ BackendType::CPU }
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
			auto range = std::views::iota(std::size_t{ 0 }, totalWork);
			
			std::for_each(std::execution::par_unseq,
				range.begin(), range.end(),
				[&](std::size_t xi) {
					sf::gl_GlobalInvocationID.x = xi;
					kernel.main();
				});
		}
	};

	
}
