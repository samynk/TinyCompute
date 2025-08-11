#pragma once
#include <limits>
#include "vec.hpp"
// ──────────────────────────────────────────────────────────────
// 1.  Kernel entry‑point concept
// ──────────────────────────────────────────────────────────────
template<typename K>
concept KernelEntry = requires(K k)
{
    { k.main() } -> std::same_as<void>;
	{
		[]() constexpr {
			return K::fileLocation;
			}()
	} -> std::convertible_to<std::string_view>;
};

template<typename K>
constexpr bool HasLocalSize = requires(K k) {
    { k.local_size } -> std::convertible_to<sf::uvec3>;
};

namespace sf
{
    // Thread‑local slot that dispatcher writes before invoking kernel
    inline thread_local sf::uvec3 gl_GlobalInvocationID(0, 0, 0);

	template<typename> struct is_vec_base_impl : std::false_type {};

	// C++20-friendly: constrain inside the value, or in the parameter list (both shown)
	template<typename T, auto N>
	struct is_vec_base_impl<vec_base<T, N>>
		: std::bool_constant< GLSLType<std::remove_cv_t<T>>&& VecSize<N> > {
	};

	// Concept: true iff the (decayed) type is vec_base<GLSLType, VecSize>
	template<typename U>
	concept VecBase =
		is_vec_base_impl<std::remove_cvref_t<U>>::value;

	// Now the “either scalar or vector” concept for your Uniform
	template<typename U>
	concept UniformValue =
		GLSLType<std::remove_cvref_t<U>> || VecBase<U>;

	template<UniformValue T, unsigned Location>
	class Uniform
	{
	public:
		Uniform()
			:m_Value{ T(0) }
		{

		}

		Uniform(T value)
			:m_Value{ value }
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

		operator const T& () const {
			return m_Value;
		}
	private:
		unsigned m_Location = Location;
		T m_Value;
	};

	template<typename T>
	class BufferResource
	{
	public:
		BufferResource() 
		{
			// no size
		}

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

		T* data()  {
			return m_Data.data();
		}


		unsigned int getSSBO_ID() const {
			return m_SSBO_ID;
		}

		void setSSBO_ID(unsigned int ssbo_id) {
			m_SSBO_ID = ssbo_id;
		}
	private:
		std::vector<T> m_Data;
		unsigned int m_SSBO_ID{ std::numeric_limits<unsigned int>::max() };
	};

	template<typename T, unsigned Binding, unsigned Set = 0 >
	class BindingPoint
	{
	public:
		BindingPoint()
			:m_pBufferData{ nullptr }
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
			return m_pBufferData->size();
		}

		BufferResource<T>* getBufferData() const{
			return m_pBufferData;
		}

		static const unsigned SET = Set;
		static const unsigned BINDING = Binding;
	private:
		BufferResource<T>* m_pBufferData;
	};
}
