#pragma once

#include <execution>
#include <algorithm>
#include <ranges>
#include <span>
#include <numeric>
#include <concepts>
#include <unordered_map>

#include "kernel_intrinsics.hpp"

namespace tc
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
			static_cast<Derived*>(this)->uploadBufferImpl(resource);
		}

		template<typename BufferType>
		void downloadBuffer(BufferResource<BufferType>& resource)
		{
			static_cast<Derived*>(this)->downloadBufferImpl(resource);
		}

		template<typename T, unsigned Binding, unsigned Set>
		void bindBuffer(const tc::BufferBinding<T, Binding, Set>& buffer)
		{
			static_cast<Derived*>(this)->bindBufferImpl(buffer);
		}

		template<tc::GPUFormat G, tc::Dim D, tc::PixelType P, unsigned B, unsigned S>
		void bindImage(const tc::ImageBinding<G, D, P, B, S> image)
		{
			static_cast<Derived*>(this)->bindImageImpl(image);
		}

		template<tc::GPUFormat G, typename P>
		void uploadImage(BufferResource<P,tc::Dim::D2>& buffer)
		{
			static_cast<Derived*>(this)->template uploadImageImpl<G,P>(buffer);
		}

		template<typename T, int Location>
		void bindUniform(const tc::Uniform<T, Location>& uniform)
		{
			static_cast<Derived*>(this)->bindUniformImpl(uniform);
		}

		template<KernelEntry K>
		void useKernel(K& k)
		{
			static_assert(HasLocalSize<K>,
				"Kernel must have a 'tc::uvec3 local_size' member.");
			static_cast<Derived*>(this)->useKernelImpl(k);
		}

		template<KernelEntry K>
		void execute(K& k, const tc::uvec3 totalWork)
		{
			static_assert(HasLocalSize<K>,
				"Kernel must have a 'tc::uvec3 local_size' member.");
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
		void uploadBufferImpl(tc::BufferResource<BufferType>& buffer)
		{
			// no op
		}

		template<typename BufferType>
		void downloadBufferImpl(tc::BufferResource<BufferType>& buffer)
		{
			// no op
		}

		template<typename T, unsigned Binding, unsigned Set>
		void bindBufferImpl(const tc::BufferBinding<T, Binding, Set>& buffer)
		{
			// no op
		}

		template<tc::GPUFormat G, tc::Dim D, PixelType P, unsigned B, unsigned S>
		void bindImageImpl(const tc::ImageBinding<G, D, P, B, S>& image)
		{
			// no op
		}

		template<GPUFormat format,typename BufferType> 
		void uploadImageImpl(tc::BufferResource<BufferType>& buffer)
		{
			// no op
		}

		template<typename T, int Location>
		void bindUniformImpl(const tc::Uniform<T, Location>& uniform)
		{
			// no op
		}

		template<KernelEntry K>
		void useKernelImpl(K& k)
		{
			// no op
		}

		inline tc::uvec3 unflatten3D(uint64_t i, tc::uvec3 dims) {
			// dims = global size (X * Y * Z total)
			uint64_t xy = uint64_t(dims.x) * dims.y;
			tc::uvec3 gid;
			gid.x = uint32_t(i % dims.x);
			gid.y = uint32_t((i / dims.x) % dims.y);
			gid.z = uint32_t(i / xy);
			return gid;
		}

		template<KernelEntry K>
		void executeImpl(K& kernel, const tc::uvec3 globalWorkSize)
		{
			if (globalWorkSize.x == 0 || globalWorkSize.y == 0 || globalWorkSize.z == 0)
				return;

			const uint64_t totalWork =
				uint64_t(globalWorkSize.x) * globalWorkSize.y * globalWorkSize.z;
			auto range = std::views::iota(uint64_t{ 0 }, totalWork);
			
			std::for_each(std::execution::par_unseq,
				range.begin(), range.end(),
				[&](const uint64_t xi) {
					tc::gl_GlobalInvocationID = unflatten3D(xi, globalWorkSize);
					kernel.main();
				});
		}
	};
}
