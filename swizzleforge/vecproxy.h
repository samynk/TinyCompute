#pragma once
#include <concepts>
#include "vec.hpp"

template<typename T>
concept arithmetic = std::integral<T> or std::floating_point<T>;


template<typename T, std::size_t N, unsigned Mask, unsigned Len>
requires arithmetic<T>
struct vec_proxy
{
	vec_base<T, N>& parent;

	// implicit read‑conversion to real vec
	constexpr operator std::remove_const_t<vec_base<T, Len>>() const
	{
		vec_base<T, Len> r;
		for (std::size_t i = 0; i < Len; ++i) {
			unsigned idx = (Mask >> ((Len - 1 - i) * 2)) & 0x3;
			r[i] = parent.data[idx];
		}
		return r;
	}

	constexpr swizzle_proxy& operator=(std::remove_const_t<vec_base<T, Len>> const& rhs)
	{
		for (std::size_t i = 0; i < Len; ++i) {
			unsigned idx = (Mask >> ((Len - 1 - i) * 2)) & 0x3;
			parent.data[idx] = rhs[i];
		}
		return *this;
	}
};

template<typename T, std::size_t N, unsigned Mask>
	requires arithmetic<T>
struct vec_proxy<T, N, Mask, 1>
{
	vec_base<T, N>& parent;

	constexpr operator std::remove_const_t<T>() const
	{
		static_assert(Len == 1, "only valid for Len==1");
		return parent.data[Mask & 0x3];
	}

};

// Primary: only for N>1
template<typename T, unsigned int N> requires arithmetic<T> struct Holder
{
	std::array<T, N> m_Data;
	// just initialize with something.
	Holder() {
		for (int idx = 0; idx < m_Data.size(); ++idx)
		{
			m_Data[idx] = idx + 1;
		}
	}

	T sum() {
		T s{ 0 };
		for (int idx = 0; idx < m_Data.size(); ++idx)
		{
			s += m_Data[idx];
		}
		return s;
	}
};

// Partial: only for N==1
template<typename T> struct Holder<T, 1>
{
	T m_Data{ 0 };

	T sum() {
		return m_Data;
	}
};
