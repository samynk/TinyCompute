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
		Float,
		Int,
		UInt,
		UNorm,
		SNorm
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

	enum class Channel { R = 0, G = 1, B = 2, A = 3 };

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
	template<typename T, int N>
	struct Pixel {
		static_assert(N >= 1 && N <= 4, "Pixel supports channel counts 1..4 only.");

		constexpr Pixel() : m_ColorData{} {}

		constexpr Pixel(T r,
			T g = channel_min<T>(),
			T b = channel_min<T>(),
			T a = channel_max<T>()) : m_ColorData{}
		{
			set<Channel::R>(r);
			set<Channel::G>(g);
			set<Channel::B>(b);
			set<Channel::A>(a);
		}

		template<Channel C>
		constexpr T get() const noexcept
		{
			constexpr int channel = static_cast<int>(C);
			if constexpr (channel < N)
				return m_ColorData[channel];
			else {
				if constexpr (C != Channel::A)
					return channel_min<T>();
				else
					return channel_max<T>();
			}
		}

		template<Channel C>
		constexpr void set(T value) noexcept
		{
			constexpr int channel = static_cast<int>(C);
			if constexpr (channel < N) {
				m_ColorData[channel] = value;
			}
		}

		template<Channel C>
		using ChannelType = T;

		static constexpr int NumChannels = N;
	private:
		std::array<T, N> m_ColorData;

	};

	using R8 = Pixel<float, 1>;
	using RA8 = Pixel<float, 2>;
	using RGB8 = Pixel<float, 3>;
	using RGBA8 = Pixel<float, 4>;

	using R8UI = Pixel<uint8_t, 1>;
	using RA8UI = Pixel<uint8_t, 2>;
	using RGB8UI = Pixel<uint8_t, 3>;
	using RGBA8UI = Pixel<uint8_t, 4>;

	template<class P, Channel C>
	concept ChannelConcept =
		requires(P p) {
		typename P::template ChannelType<C>;

		{ p.template get<C>() }
		-> std::convertible_to<typename P::template ChannelType<C>>;

		{ p.template set<C>(p.template get<C>()) };
	};

	template<class P>
	concept PixelConcept =
		ChannelConcept<P, Channel::R>&&
		ChannelConcept<P, Channel::G>&&
		ChannelConcept<P, Channel::B>&&
		ChannelConcept<P, Channel::A>;
}