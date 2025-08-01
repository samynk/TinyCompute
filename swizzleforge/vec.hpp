#pragma once

#include <array>
#include <iostream>
#include <concepts>
#include "swizzle.hpp"


namespace sf {
	// concepts
	template<std::size_t N>
	concept VecSize = (N == 2 || N == 3 || N == 4);

	template<typename T>
	concept GLSLFloatType = std::same_as<T, float> || std::same_as<T, double>;

	template<typename T>
	concept GLSLIntegerType = std::same_as<T, std::int32_t> || std::same_as<T, std::uint32_t>;

	template<typename T>
	concept GLSLNumericType = GLSLFloatType<T> || GLSLIntegerType<T>;

	template<typename T>
	concept GLSLType = GLSLNumericType<T> || std::same_as<T, bool>;



	template<typename T, std::size_t N>
		requires GLSLType<T> and VecSize<N>
	class vec_base {
		// primary template ⇒ any non‑vector type counts as 1
		template<class U>
		struct comp_count :std::integral_constant<std::size_t, 1> {};

		// partial specialization ⇒ a vector contributes its dimension N
		template<typename IT, std::size_t IN>
		struct comp_count<vec_base<IT, IN>> : std::integral_constant<std::size_t, IN> {};

		// scalar types are not vecs
		template<class IT> struct is_vec : std::false_type {};
		// vec_base are vecs
		template<class IT, std::size_t IN> struct is_vec<sf::vec_base<IT, IN>> : std::true_type {};

		template<class U>
		static const bool is_vec_v = is_vec<std::remove_cvref_t<U>>::value;


		// convenience variable template
		template<class U>
		static const std::size_t comp_count_v = comp_count<std::remove_cvref_t<U>>::value;


	public:
		struct EmptyType {};
		using T1 = typename std::conditional_t<N >= 1, T, EmptyType>;
		using T2 = typename std::conditional_t<N >= 2, T, EmptyType>;
		using T3 = typename std::conditional_t<N >= 3, T, EmptyType>;
		using T4 = typename std::conditional_t<N >= 4, T, EmptyType>;

		static constexpr std::size_t size = N;

		union {
			struct { T1 x; T2 y; T3 z; T4 w; };
			std::array<T, N> data;
		};

		// Default constructor
		constexpr vec_base() = default;

		template<typename S>
			requires std::convertible_to<S, T>
		constexpr vec_base(S value) {
			data.fill(static_cast<T>(value));
		}

		template<class... Args>
			requires ((comp_count_v<Args> +...) == N)
		constexpr explicit vec_base(const Args&... args)
		{
			std::size_t cursor = 0;
			([&]
				{
					auto part = flatten_arg<T>(args);
					for (T v : part) data[cursor++] = v;
				}(), ...);
		}


		// rvalue
		constexpr T& operator[](std::size_t i) {
			assert(i < N);
			return data[i];
		}

		constexpr const T& operator[](std::size_t i) const {
			assert(i < N);
			return data[i];
		}

		template<unsigned Mask, unsigned Len>
		constexpr auto operator[](const SwizzleMask<Mask, Len>) const {
			vec_base<T, Len> r;
			for (std::size_t i = 0; i < Len; ++i) {
				unsigned idx = (Mask >> ((Len - 1 - i) * 2)) & 0x3;
				r[i] = data[idx];
			}
			return r;
		}

		template<unsigned Mask>
		constexpr auto operator[](const SwizzleMask<Mask, 1>) const {
			T r = data[Mask & 0x3];
			return r;
		}

		constexpr vec_base<bool, N> operator==(const vec_base<T, N>& other) const {
			vec_base<bool, N> result;
			for (std::size_t i = 0; i < data.size(); ++i)
			{
				result[i] = (data[i] == other.data[i]);
			}
			return result;
		}

		// Debug print
		friend std::ostream& operator<<(std::ostream& os, const vec_base& v) {
			os << "(";
			for (std::size_t i = 0; i < N; ++i) {
				os << v.data[i];
				if (i != N - 1) os << ", ";
			}
			os << ")";
			return os;
		}

		template<typename IT, std::size_t IN, unsigned Mask, unsigned Len>
		struct vec_proxy
		{
			vec_base<IT, IN>& parent;

			// implicit read‑conversion to real vec
			constexpr operator std::remove_const_t<vec_base<IT, Len>>() const
			{
				vec_base<IT, Len> r;
				for (std::size_t i = 0; i < Len; ++i) {
					unsigned idx = (Mask >> ((Len - 1 - i) * 2)) & 0x3;
					r[i] = parent.data[idx];
				}
				return r;
			}

			constexpr vec_proxy& operator=(std::remove_const_t<vec_base<IT, Len>> const& rhs)
			{
				for (std::size_t i = 0; i < Len; ++i) {
					unsigned idx = (Mask >> ((Len - 1 - i) * 2)) & 0x3;
					parent.data[idx] = rhs[i];
				}
				return *this;
			}
		};

		template<typename IT, std::size_t IN, unsigned Mask>
		struct vec_proxy<IT, IN, Mask, 1>
		{
			vec_base<T, N>& parent;

			constexpr operator std::remove_const_t<IT>() const
			{
				return parent.data[Mask & 0x3];
			}

			constexpr vec_proxy& operator=(IT value)
			{
				parent.data[Mask & 0x3] = value;
				return *this;
			}

		};

		// non-const vector → mutable proxy
		template<unsigned Mask, unsigned Len>
		constexpr auto operator[](SwizzleMask<Mask, Len>)
			-> vec_proxy<T, N, Mask, Len>
		{
			return { *this };
		}



	private:
		template<class IT, class Arg>
		constexpr auto flatten_arg(const Arg& a)
		{
			if constexpr (is_vec_v<Arg>)
			{
				std::array<IT, std::remove_cvref_t<Arg>::size> out{};
				for (std::size_t i = 0; i < out.size(); ++i) out[i] = static_cast<IT>(a[i]);
				return out;
			}
			else
			{
				return std::array<IT, 1>{ static_cast<IT>(a) };
			}
		}
	};


	


	// dot product ---------------------------------------------------
	template<typename T, std::size_t N>
		requires VecSize<N>&& GLSLFloatType<T>
	T dot(const vec_base<T, N>& a, const vec_base<T, N>& b)
	{
		T sum = 0;
		for (std::size_t i = 0; i < N; ++i) sum += a[i] * b[i];
		return sum;
	}

	template<typename T, std::size_t N>
		requires VecSize<N>&& std::same_as<T, bool>
	bool all(const vec_base<T, N>& v) noexcept {
		for (std::size_t i = 0; i < N; ++i) {
			if (!v[i]) return false;
		}
		return true;
	}

	template<typename T, std::size_t N>
		requires VecSize<N>&& std::same_as<T, bool>
	bool any(const vec_base<T, N>& v) noexcept {
		for (std::size_t i = 0; i < N; ++i) {
			if (v[i]) return true;
		}
		return false;
	}

	

	using vec2 = sf::vec_base<float, 2>;
	using vec3 = sf::vec_base<float, 3>;
	using vec4 = sf::vec_base<float, 4>;

	using dvec2 = sf::vec_base<double, 2>;
	using dvec3 = sf::vec_base<double, 3>;
	using dvec4 = sf::vec_base<double, 4>;

	using integer = std::int32_t;
	using ivec2 = sf::vec_base<std::int32_t, 2>;
	using ivec3 = sf::vec_base<std::int32_t, 3>;
	using ivec4 = sf::vec_base<std::int32_t, 4>;

	using uint = std::uint32_t;
	using uvec2 = sf::vec_base<std::uint32_t, 2>;
	using uvec3 = sf::vec_base<std::uint32_t, 3>;
	using uvec4 = sf::vec_base<std::uint32_t, 4>;

	using bvec2 = sf::vec_base<bool, 2>;
	using bvec3 = sf::vec_base<bool, 3>;
	using bvec4 = sf::vec_base<bool, 4>;
}

