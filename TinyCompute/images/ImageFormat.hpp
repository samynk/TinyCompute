#pragma once
#include <limits>
#include <type_traits>
#include <array>

namespace tc {
	enum class InternalFormat {
		RGBA32F,
		R32F,
		RGBA8,
		R8UI,
		RGB8UI
	};

	enum class Scalar {
		FLOAT,
		INT,
		UINT,
		UNORM,
		SNORM
	};

	enum class Dim : uint8_t {
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

	enum class Channel : std::uint8_t { R, G, B, A };

	template<typename T>
	constexpr T channel_min() noexcept {
		if constexpr (std::is_floating_point_v<T>) return T(0);
		else return std::numeric_limits<T>::min();
	}

	template<typename T>
	constexpr T channel_max() noexcept {
		if constexpr (std::is_floating_point_v<T>) return T(1);
		else return std::numeric_limits<T>::max();
	}
}

namespace tc::cpu {
	template<typename T, uint8_t N>
	struct Pixel {
		static_assert(N >= 1 && N <= 4, "Pixel supports channel counts 1..4 only.");
	};

	template<typename T>
	struct Pixel<T, 1>
	{
		T r;

		template<Channel C>
		constexpr T get() const noexcept
		{
			if constexpr (C != Channel::A) return r;
			else return channel_max<T>();
		}

		template<Channel C>
		constexpr void set(T value) noexcept
		{
			if constexpr (C != Channel::A) r = value;
			// ignore A
		}
		static constexpr uint8_t NumChannels = 1;
		using ChannelType = T;
	};

	template<typename T>
	struct Pixel<T, 2>
	{
		T r, a;

		template<Channel C>
		constexpr T get() const noexcept
		{
			if constexpr (C != Channel::A) return r;
			else return a;
		}

		template<Channel C>
		constexpr void set(T value) noexcept
		{
			if constexpr (C != Channel::A) r = value;
			else a = value;
		}

		static constexpr uint8_t NumChannels = 2;
		using ChannelType = T;
	};

	template<typename T>
	struct Pixel<T, 3>
	{
		T r, g, b;

		template<Channel C>
		constexpr T get() const noexcept
		{
			if constexpr (C == Channel::R) return r;
			else if constexpr (C == Channel::G) return g;
			else if constexpr (C == Channel::B) return b;
			else return channel_max<T>();
		}

		template<Channel C>
		constexpr void set(T value) noexcept
		{
			if constexpr (C == Channel::R) r = value;
			else if constexpr (C == Channel::G) g = value;
			else if constexpr (C == Channel::B) b = value;
			// don't write alpha.
		}

		static constexpr uint8_t NumChannels = 3;
		using ChannelType = T;
	};

	template<typename T>
	struct Pixel<T, 4>
	{
		T r, g, b, a;

		template<Channel C>
		constexpr T get() const noexcept
		{
			if constexpr (C == Channel::R) return r;
			else if constexpr (C == Channel::G) return g;
			else if constexpr (C == Channel::B) return b;
			else return a;
		}

		template<Channel C>
		constexpr void set(T value) noexcept
		{
			if constexpr (C == Channel::R) r = value;
			else if constexpr (C == Channel::G) g = value;
			else if constexpr (C == Channel::B) b = value;
			else a = value;
		}

		static constexpr uint8_t NumChannels = 4;
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

	using R8 = Pixel<float, 1>;
	using RA8 = Pixel<float, 2>;
	using RGB8 = Pixel<float, 3>;
	using RGBA8 = Pixel<float, 4>;

	using R8UI = Pixel<uint8_t, 1>;
	using RA8UI = Pixel<uint8_t, 2>;
	using RGB8UI = Pixel<uint8_t, 3>;
	using RGBA8UI = Pixel<uint8_t, 4>;
}