#pragma once
#include <limits>
#include <vector>
#include <concepts>
#include <string_view>
#include <stdexcept>
#include <algorithm>
#include <random>

#include "vec.hpp"
#include "images/ImageFormat.hpp"
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
	{ k.local_size } -> std::convertible_to<tc::uvec3>;
};

namespace tc
{
	// Thread‑local slot that dispatcher writes before invoking kernel
	inline thread_local tc::uvec3 gl_GlobalInvocationID(0, 0, 0);

	template<typename> struct is_vec_base_impl : std::false_type {};

	// C++20-friendly: constrain inside the value, or in the parameter list (both shown)
	template<typename T, auto N>
	struct is_vec_base_impl<vec_base<T, N>>
		: std::bool_constant< GLSLType<std::remove_cv_t<T>>&& VecSize<N> > {
	};

	// Concept: true if the (decayed) type is vec_base<GLSLType, VecSize>
	template<typename U>
	concept VecBase =
		is_vec_base_impl<std::remove_cvref_t<U>>::value;

	template<typename U>
	concept UniformValue =
		GLSLType<std::remove_cvref_t<U>> || VecBase<U>;


	template<tc::Dim D>
	struct DimTraits;

	// Dimension 1 --> use uint (uint32_t)
	template<>
	struct DimTraits<tc::Dim::D1> {
		using IndexType = int32_t;

		static constexpr int32_t product(IndexType x) {
			return x;
		}

		static constexpr int32_t coordinateToIndex(IndexType coord, IndexType /*size*/) {
			return coord;
		}
	};

	// Dimension 2 --> use uvec2
	template<>
	struct DimTraits<tc::Dim::D2> {
		using IndexType = tc::ivec2;

		static constexpr int32_t product(IndexType dim) {
			return dim.x * dim.y;
		}

		static constexpr int32_t coordinateToIndex(IndexType coord, IndexType size) {
			return coord.y * size.x + coord.x;
		}
	};

	// Dimension 3 --> use uvec3
	template<>
	struct DimTraits<tc::Dim::D3> {
		using IndexType = tc::ivec3;

		static constexpr int32_t product(IndexType dim) {
			return dim.x * dim.y * dim.z;
		}

		static constexpr int32_t coordinateToIndex(IndexType coord, IndexType size) {
			return (coord.z * size.y + coord.y) * size.x + coord.x;
		}
	};

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

	template<typename T, tc::Dim D = tc::Dim::D1>
	class BufferResource
	{
	public:
		using dimType = typename DimTraits<D>::IndexType;

		BufferResource()
			:m_BufferSize{ 0 }
		{
			// no size
		}

		BufferResource(dimType bufferSize)
			:m_BufferSize(bufferSize),
			m_Data(Traits::product(bufferSize))
		{
		}

		const T& operator[](dimType index) const
		{
			return m_Data[Traits::coordinateToIndex(index, m_BufferSize)];
		}

		T& operator[](dimType index)
		{
			return m_Data[Traits::coordinateToIndex(index, m_BufferSize)];
		}


		size_t size() const {
			return m_Data.size();
		}

		T* data() {
			return m_Data.data();
		}

		unsigned int getSSBO_ID() const {
			return m_SSBO_ID;
		}

		void setSSBO_ID(unsigned int ssbo_id) {
			m_SSBO_ID = ssbo_id;
		}

		dimType getDimension() const {
			return m_BufferSize;
		}

		void swap(BufferResource& other) noexcept
		{
			using std::swap;
			swap(m_BufferSize, other.m_BufferSize);
			swap(m_SSBO_ID, other.m_SSBO_ID);
			swap(m_Data, other.m_Data);
		}

		friend void swap(BufferResource& a, BufferResource& b) noexcept(noexcept(a.swap(b))) {
			a.swap(b);
		}

		void randomize(T min, T max) requires std::is_arithmetic_v<T> {
			std::mt19937 rng{ std::random_device{}() };

			if constexpr (std::is_integral_v<T>) {
				std::uniform_int_distribution<T> dist(min, max);
				std::generate(m_Data.begin(), m_Data.end(),
					[&]() { return dist(rng); });
			}
			else {
				std::uniform_real_distribution<T> dist(min, max);
				std::generate(m_Data.begin(), m_Data.end(),
					[&]() { return dist(rng); });
			}
		}

	private:
		using Traits = DimTraits<D>;
		dimType m_BufferSize;
		std::vector<T> m_Data;
		unsigned int m_SSBO_ID{ 0 };
	};


	template<typename T, unsigned Binding, unsigned Set = 0 >
	class BufferBinding
	{
	public:
		BufferBinding()
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

		BufferResource<T>* getBufferData() const {
			return m_pBufferData;
		}

		template<unsigned B2, unsigned S2>
		friend void swap(BufferBinding& a, BufferBinding<T, B2, S2>& b) noexcept {
			using std::swap;
			swap(*a.getBufferData(), *b.getBufferData());
		}

		static const unsigned SET = Set;
		static const unsigned BINDING = Binding;
	private:
		BufferResource<T>* m_pBufferData;

		template<typename T, unsigned B1, unsigned S1>
		friend class BufferBinding;
	};

	template<tc::InternalFormat G>
	struct GPUFormatTraits;

	template<>
	struct GPUFormatTraits<tc::InternalFormat::RGBA32F> {
		using ChannelType = float;
		using VectorType = tc::vec4;
		static constexpr uint8_t NumChannels = 4;
		static inline constexpr tc::Channel indices[NumChannels] = { Channel::R, Channel::G, Channel::B, Channel::A };
	};

	template<>
	struct GPUFormatTraits<tc::InternalFormat::RGBA8> {
		using ChannelType = float;
		using VectorType = tc::vec4;
		static constexpr uint8_t NumChannels = 4;
		static inline constexpr tc::Channel indices[NumChannels] = { Channel::R, Channel::G, Channel::B, Channel::A };
	};

	template<>
	struct GPUFormatTraits<tc::InternalFormat::R8UI> {
		using ChannelType = uint8_t;
		using VectorType = tc::uvec4;
		static inline constexpr std::array<tc::Channel,1> channels{ Channel::R };
	};

	template<tc::InternalFormat G, tc::Dim D, tc::cpu::PixelConcept pixType, unsigned Binding, unsigned Set = 0 >
	class ImageBinding
	{
	public:
		ImageBinding()
			:m_pBufferData{ nullptr }
		{

		}

		void attach(BufferResource<pixType, D>* pData) {
			m_pBufferData = pData;
		}

		unsigned size() const {
			return m_pBufferData->size();
		}

		BufferResource<pixType, D>* getBufferData() const {
			return m_pBufferData;
		}

		inline static constexpr tc::InternalFormat GPUFormat = G;
		inline static constexpr tc::Dim Dimension = D;
		inline static constexpr unsigned BINDING = Binding;
		inline static constexpr unsigned SET = Set;
	private:
		BufferResource<pixType, D>* m_pBufferData;
	};

	template<class Src, class Dst>
	struct ChannelConverter;

	template<class T>
	struct ChannelConverter<T, T> {
		static constexpr T apply(T v) noexcept { return v; }
	};

	template<>
	struct ChannelConverter<std::uint8_t, float> {
		static constexpr float apply(std::uint8_t v) noexcept {
			return float(v) * (1.0f / 255.0f);
		}
	};

	template<>
	struct ChannelConverter<float, std::uint8_t> {
		static constexpr uint8_t apply(float v) noexcept {
			return static_cast<uint8_t>(std::clamp(v, 0.0f, 100.0f) * 255.0f + 0.5f);
		}
	};

	template<Dim D>
	inline constexpr unsigned dim_count_v =
		(D == Dim::D1 ? 1u :
			D == Dim::D2 ? 2u :
			/*D3*/          3u);

	// Convenience alias for the uv type
	template<Dim D>
	using tcVec = tc::vec_base<int32_t, dim_count_v<D>>;

	template<Channel C, class Dst, tc::cpu::PixelConcept P>
	constexpr Dst channelLoad(const P& px) noexcept {
		using Src = typename P::template ChannelType<C>;
		return ChannelConverter<Src, Dst>::apply(px.template get<C>());
	}

	template<tc::InternalFormat G, tc::Dim D, tc::cpu::PixelConcept P, unsigned B, unsigned S>
	auto imageLoad(const ImageBinding<G, D, P, B, S>& image, tcVec<D> texCoord)
	{
		const auto* buf = image.getBufferData();
		if (!buf) throw std::runtime_error("imageLoad: no buffer attached");

		const P& px = (*buf)[texCoord];

		using gpuTraits = GPUFormatTraits<G>;
		using dst_t = typename gpuTraits::ChannelType;
		using vec_t = typename gpuTraits::VectorType;
		vec_t result{
			channelLoad<Channel::R,dst_t,P>(px),
			channelLoad<Channel::G,dst_t,P>(px),
			channelLoad<Channel::B,dst_t,P>(px),
			channelLoad<Channel::A,dst_t,P>(px)
		};
		return result;
	}

	template<tc::InternalFormat G, tc::Dim D, tc::cpu::PixelConcept P, unsigned B, unsigned S>
	tcVec<D> imageSize(const ImageBinding<G, D, P, B, S>& image)
	{
		return image.getBufferData()->getDimension();
	}

	template<Channel C, class Src, tc::cpu::PixelConcept P>
	constexpr void channelStore(P& px, Src value) noexcept {
		using dst_t = typename P::template ChannelType<C>;
		px.template set<C>(ChannelConverter<Src,dst_t>::apply(value));
	}

	template<tc::InternalFormat G, tc::Dim D, tc::cpu::PixelConcept P, unsigned B, unsigned S>
	void imageStore(
		const ImageBinding<G, D, P, B, S>& image,
		tcVec<D> texCoord,
		typename GPUFormatTraits<G>::VectorType value
	)
	{
		auto* buf = image.getBufferData();
		if (!buf) throw std::runtime_error("imageStore: no buffer attached");

		using src_t = typename GPUFormatTraits<G>::ChannelType;
		auto& px = (*buf)[texCoord];
		channelStore<Channel::R,src_t, P>(px,value.x);
		channelStore<Channel::G,src_t, P>(px,value.y);
		channelStore<Channel::B,src_t, P>(px,value.z);
		channelStore<Channel::A,src_t, P>(px,value.w);
	}
}
