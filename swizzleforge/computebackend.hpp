#pragma once

#include <execution>
#include <algorithm>
#include <ranges>
#include <span>
#include <numeric>
#include <concepts>

namespace sf
{
	enum class BackendType {
		CPU, OpenGL
	};

	enum class AccessType {
		READ, WRITE, READWRITE
	};

	template<GLSLType T, unsigned Binding>
	class Uniform
	{
	public:
		Uniform()
			:m_Value{T(0)}
		{

		}

		Uniform(T value)
			:m_Value{value}
		{

		}

		T& get() {
			return m_Value;
		}

		operator T& () {
			return m_Value;
		}

		const T& get() const {
			return m_Value;
		}

		operator const T&() const {
			return m_Value;
		}
	private:
		unsigned m_BindingId = Binding;
		T m_Value;
	};

	template<typename T>
	class BufferResource
	{
	public:
		BufferResource(unsigned size) 
		{
			m_Data.resize(size);
		}

		const T& operator[](unsigned idx) const
		{
			return m_Data[idx];
		}

		T& operator[](unsigned idx)
		{
			return m_Data[idx];
		}

		unsigned size() const {
			return m_Data.size();
		}
	private:
		std::vector<T> m_Data;
	};

	template<typename T, unsigned Set, unsigned Binding>
	class BindingPoint
	{
	public:
		BindingPoint()
			:m_pBufferData{nullptr}
		{

		}

		const T& operator[](unsigned idx) const
		{
			return (*m_pBufferData)[idx];
		}

		T& operator[](unsigned idx) 
		{
			return (*m_pBufferData)[idx];
		}
		
		void attach(BufferResource<T>* pData) {
			m_pBufferData = pData;
		}

		unsigned size() const {
			return m_pBufferData.size();
		}

		BufferResource<T>* getBufferData() {
			return m_pBufferData;
		}

		static const unsigned SET = Set;
		static const unsigned BINDING = Binding;
	private:
		BufferResource<T>* m_pBufferData;
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
					kernel();
				});
		}
	};
}
