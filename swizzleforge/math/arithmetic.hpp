#pragma once

#include "../vec.hpp"

namespace tc {
	// numeric operators
	// + operator
	template<typename T, std::size_t N>
		requires GLSLNumericType<T>
	constexpr tc::vec_base<T, N> operator+(tc::vec_base<T, N> lhs, tc::vec_base<T, N> rhs)
	{
		for (std::size_t i = 0; i < N; ++i) lhs[i] += rhs[i];
		return lhs;
	}

	template<typename T, std::size_t N, typename S>
		requires GLSLNumericType<T> and std::convertible_to<S, T>
	constexpr tc::vec_base<T, N> operator+(tc::vec_base<T, N> v, S s)
	{
		return v + tc::vec_base<T, N>(static_cast<T>(s));
	}

	template<typename T, std::size_t N, typename S>
		requires  GLSLNumericType<T> and std::convertible_to<S, T>
	constexpr tc::vec_base<T, N> operator+(S s, tc::vec_base<T, N> v)
	{
		return v + tc::vec_base<T, N>(static_cast<T>(s));
	}

	template<typename T, std::size_t N>
	constexpr tc::vec_base<T, N> operator*(tc::vec_base<T, N> lhs, tc::vec_base<T, N> rhs)
	{
		for (std::size_t i = 0; i < N; ++i) lhs[i] *= rhs[i];
		return lhs;
	}

	// scalar broadcast of product ----------------------------------------------
	template<typename T, std::size_t N, typename S>
		requires std::convertible_to<S, T>
	constexpr tc::vec_base<T, N> operator*(tc::vec_base<T, N> v, S s)
	{
		return v * tc::vec_base<T, N>(static_cast<T>(s));
	}

	template<typename T, std::size_t N>
	constexpr tc::vec_base<T, N> operator-(tc::vec_base<T, N> lhs, tc::vec_base<T, N> rhs)
	{
		for (std::size_t i = 0; i < N; ++i) lhs[i] -= rhs[i];
		return lhs;
	}
}