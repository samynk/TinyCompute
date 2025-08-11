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
		void uploadBuffer(BufferResource<BufferType>& resource )
		{
			static_cast<Derived*>(this)->uploadImpl(resource);
		}

		template<typename BufferType>
		void downloadBuffer(BufferResource<BufferType>& resource)
		{
			static_cast<Derived*>(this)->downloadImpl(resource);
		}

		template<typename T, unsigned Binding, unsigned Set>
		void bindBuffer(const sf::BindingPoint<T, Binding, Set>& buffer)
		{
			static_cast<Derived*>(this)->bindBuffer(buffer);
		}

		template<typename T, unsigned Location>
		void bindUniform(const sf::Uniform<T, Location>& uniform)
		{
			static_cast<Derived*>(this)->bindUnfirom(uniform);
		}

		template<KernelEntry K>
		void useKernel(K& k)
		{
			static_assert(HasLocalSize<K>,
				"Kernel must have a 'sf::uvec3 local_size' member.");
			static_cast<Derived*>(this)->useKernel(k);
		}

		template<KernelEntry K>
		void execute(K& k, const sf::uvec3 totalWork)
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
		void uploadBuffer(BufferResource<BufferType>& buffer)
		{
			// no op
		}

		template<typename BufferType>
		void downloadBuffer(BufferResource<BufferType>& buffer)
		{
			// no op
		}

		template<typename T, unsigned Binding, unsigned Set>
		void bindBuffer(const sf::BindingPoint<T, Binding, Set>& buffer)
		{
			// no op
		}

		template<typename T, unsigned Location>
		void bindUniform(const sf::Uniform<T, Location>& uniform)
		{
			// no op
		}

		template<KernelEntry K>
		void useKernel(K& k)
		{
			// no op
		}

		inline sf::uvec3 unflatten3D(uint64_t i, sf::uvec3 dims) {
			// dims = global size (X * Y * Z total)
			uint64_t xy = uint64_t(dims.x) * dims.y;
			sf::uvec3 gid;
			gid.x = uint32_t(i % dims.x);
			gid.y = uint32_t((i / dims.x) % dims.y);
			gid.z = uint32_t(i / xy);
			return gid;
		}

		template<KernelEntry K>
		void executeImpl(K& kernel, const sf::uvec3 globalWorkSize)
		{
			if (globalWorkSize.x == 0 || globalWorkSize.y == 0 || globalWorkSize.z == 0)
				return;

			const uint64_t totalWork =
				uint64_t(globalWorkSize.x) * globalWorkSize.y * globalWorkSize.z;
			auto range = std::views::iota(uint64_t{ 0 }, totalWork);
			
			std::for_each(std::execution::par_unseq,
				range.begin(), range.end(),
				[&](const uint64_t xi) {
					sf::gl_GlobalInvocationID = unflatten3D(xi, globalWorkSize);
					kernel.main();
				});
		}
	};
}
