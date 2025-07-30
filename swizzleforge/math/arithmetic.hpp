#pragma once

#include "vec.hpp"

namespace sf {
	// numeric operators
	// + operator
	template<typename T, std::size_t N>
		requires GLSLNumericType<T>
	constexpr sf::vec_base<T, N> operator+(sf::vec_base<T, N> lhs, sf::vec_base<T, N> rhs)
	{
		for (std::size_t i = 0; i < N; ++i) lhs[i] += rhs[i];
		return lhs;
	}

	template<typename T, std::size_t N, typename S>
		requires GLSLNumericType<T> and std::convertible_to<S, T>
	constexpr sf::vec_base<T, N> operator+(sf::vec_base<T, N> v, S s)
	{
		return v + sf::vec_base<T, N>(static_cast<T>(s));
	}

	template<typename T, std::size_t N, typename S>
		requires  GLSLNumericType<T> and std::convertible_to<S, T>
	constexpr sf::vec_base<T, N> operator+(S s, sf::vec_base<T, N> v)
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
}