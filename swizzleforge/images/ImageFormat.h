#pragma once


namespace tc {
	enum class GPUFormat {
		R32F,
		RGBA8
	};

	enum class Dim : uint32_t {
		D1 = 1u,
		D2 = 2u,
		D3 = 3u
	};

	enum class ImageFormat
	{
		RED,
		RG,
		RGB,
		BGR,
		RGBA,
		BGRA,
		RED_INTEGER,
		RG_INTEGER,
		RGB_INTEGER,
		BGR_INTEGER,
		RGBA_INTEGER,
		BGRA_INTEGER,
		STENCIL_INDEX,
		DEPTH_COMPONENT,
		DEPTH_STENCIL
	};


	template<typename T, uint8_t N>
	struct Pixel {
		struct EmptyType {};
		using T1 = typename std::conditional_t<N >= 1, T, EmptyType>;
		using T2 = typename std::conditional_t<N >= 2, T, EmptyType>;
		using T3 = typename std::conditional_t<N >= 3, T, EmptyType>;
		using T4 = typename std::conditional_t<N >= 4, T, EmptyType>;

		union {
			struct { T1 r; T2 g; T3 b; T4 a; };
			std::array<T, N> data;
		};

		T operator[](uint32_t idx) const{
			return data[idx];
		}

		static constexpr uint8_t NumChannels = N;
		using ChannelType = T;
	};

	// Pixel concept
	template<typename X>
	struct is_pixel_specialization : std::false_type {};

	// true only for Pixel<T,N>
	template<typename T, std::size_t N>
	struct is_pixel_specialization<Pixel<T, N>> : std::true_type {};

	template<typename X>
	inline constexpr bool is_pixel_specialization_v =
		is_pixel_specialization<std::remove_cvref_t<X>>::value;

	// Friendly concept name for errors
	template<typename P>
	concept PixelType = is_pixel_specialization_v<P>;

	using R8 = Pixel<uint8_t, 1>;
	using RG8 = Pixel<uint8_t, 2>;
	using RGB8 = Pixel<uint8_t, 3>;
	using RGBA8 = Pixel<uint8_t, 4>;


}