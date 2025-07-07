#pragma once

#include <array>
#include <iostream>
#include "swizzle.hpp"


namespace sf {
	// concepts
	template<std::size_t N>
	concept VecSize = (N == 2 || N == 3 || N == 4);

	template<typename V>
	concept FloatVec = std::floating_point<typename V::value_type>;


	template<typename T, std::size_t N>
	class vec_base {
		// primary template ⇒ any non‑vector type counts as 1
		template<class U>
		struct comp_count :std::integral_constant<std::size_t, 1> {};

		// partial specialisation ⇒ a vector contributes its dimension N
		template<typename T,std::size_t N>
		struct comp_count<vec_base<T, N>> : std::integral_constant<std::size_t, N> {};

		// scalar types are not vecs
		template<class T> struct is_vec : std::false_type {};
		// vec_base are vecs
		template<class T, std::size_t M> struct is_vec<sf::vec_base<T, M>> : std::true_type {};

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

		// Variadic constructor for e.g. Vec<float,3> v(1.0f, 2.0f, 3.0f);
		/*template<typename... Args,
			typename = std::enable_if_t<sizeof...(Args) == N>>
			constexpr vec_base(Args... args) : data{ static_cast<T>(args)... } {}*/

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


			// Index access
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

		constexpr vec_base<bool,N> operator==(const vec_base<T, N>& other) const {
			vec_base<bool, N> result;
			for (std::size_t i = 0; i < data.size(); ++i)
			{
				result[i] = (data[i] != other.data[i]);
			}
		}

		constexpr bool operator!=(const vec_base<T, N>& other) const {
			return !(*this == other);
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

		template<typename T, std::size_t N, unsigned Mask, unsigned Len>
		struct swizzle_proxy
		{
			vec_base<T, N>& parent;

			// implicit read‑conversion to real vec
			operator vec_base<T, Len>() const
			{
				vec_base<T, Len> r;
				for (std::size_t i = 0; i < Len; ++i) {
					unsigned idx = (Mask >> ((Len - 1 - i) * 2)) & 0x3;
					r[i] = parent.data[idx];
				}
				return r;
			}

			// write from another vec
			swizzle_proxy& operator=(const vec_base<T, Len>& rhs)
			{
				for (std::size_t i = 0; i < Len; ++i) {
					unsigned idx = (Mask >> ((Len - 1 - i) * 2)) & 0x3;
					parent.data[idx] = rhs[i];
				}
				return *this;
			}

			// write from scalar (broadcast)
			template<std::convertible_to<T> S>
			swizzle_proxy& operator=(S s)
			{
				for (std::size_t i = 0; i < Len; ++i) {
					unsigned idx = (Mask >> ((Len - 1 - i) * 2)) & 0x3;
					parent.data[idx] = static_cast<T>(s);
				}
				return *this;
			}
		};


		template<unsigned Mask, unsigned Len>
		constexpr auto operator[](const SwizzleMask<Mask, Len>)
			-> swizzle_proxy<T, N, Mask, Len>
		{
			return { *this };
		}

private:
			template<class T, class Arg>
				
			constexpr auto flatten_arg(const Arg& a)
			{
				if constexpr (is_vec_v<Arg>)
				{
					std::array<T, std::remove_cvref_t<Arg>::size> out{};
					for (std::size_t i = 0; i < out.size(); ++i) out[i] = static_cast<T>(a[i]);
					return out;
				}
				else
				{
					return std::array<T, 1>{ static_cast<T>(a) };
				}
			}
	};


	// numeric operators
	// vector + vector (same N) --------------------------------------
	template<typename T, std::size_t N>
	constexpr sf::vec_base<T, N> operator+(sf::vec_base<T, N> lhs, sf::vec_base<T, N> rhs)
	{
		for (std::size_t i = 0; i < N; ++i) lhs[i] += rhs[i];
		return lhs;
	}

	// scalar broadcast ----------------------------------------------
	template<typename T, std::size_t N, typename S>
		requires std::convertible_to<S, T>
	constexpr sf::vec_base<T, N> operator+(sf::vec_base<T, N> v, S s)
	{
		return v + sf::vec_base<T, N>(static_cast<T>(s));
	}

	template<typename T, std::size_t N>
	constexpr sf::vec_base<T, N> operator*(sf::vec_base<T, N> lhs, sf::vec_base<T, N> rhs)
	{
		for (std::size_t i = 0; i < N; ++i) lhs[i] *= rhs[i];
		return lhs;
	}

	// scalar broadcast of product ----------------------------------------------
	template<typename T, std::size_t N, typename S>
		requires std::convertible_to<S, T>
	constexpr sf::vec_base<T, N> operator*(sf::vec_base<T, N> v, S s)
	{
		return v * sf::vec_base<T, N>(static_cast<T>(s));
	}

	template<typename T, std::size_t N>
	constexpr sf::vec_base<T, N> operator-(sf::vec_base<T, N> lhs, sf::vec_base<T, N> rhs)
	{
		for (std::size_t i = 0; i < N; ++i) lhs[i] -= rhs[i];
		return lhs;
	}



	// dot product ---------------------------------------------------
	template<typename T, std::size_t N>
		requires VecSize<N>&& FloatVec<T>
	constexpr typename T dot(const vec_base<T, N>& a, const vec_base<T, N>& b)
	{
		T sum = 0;
		for (std::size_t i = 0; i < N; ++i) sum += a[i] * b[i];
		return sum;
	}





	using vec2 = vec_base<float, 2>;
	using vec3 = vec_base<float, 3>;
	using vec4 = vec_base<float, 4>;

	using ivec2 = vec_base<int, 2>;
	using ivec3 = vec_base<int, 3>;
	using ivec4 = vec_base<int, 4>;

	using uvec2 = vec_base<unsigned int, 2>;
	using uvec3 = vec_base<unsigned int, 3>;
	using uvec4 = vec_base<unsigned int, 4>;

	using bvec2 = vec_base<bool, 2>;
	using bvec3 = vec_base<bool, 3>;
	using bvec4 = vec_base<bool, 4>;
}

