#pragma once

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

	template<GLSLType T, unsigned Binding>
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

		BufferResource<T>* getBufferData() {
			return m_pBufferData;
		}

		static const unsigned SET = Set;
		static const unsigned BINDING = Binding;
	private:
		BufferResource<T>* m_pBufferData;
	};
}
