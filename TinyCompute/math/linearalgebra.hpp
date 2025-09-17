#pragma once
#include "../vec.hpp"
#include "arithmetic.hpp"
namespace tc
{
	template<typename T, uint8_t N>
		requires GLSLFloatType<T> and VecSize<N>
	tc::vec_base<T, N> normalize(tc::vec_base<T, N> op)
	{
		// minimum is two components
		T squared = op.x * op.x + op.y * op.y;
		if constexpr (N >= 3) {
			squared += op.z * op.z;
		}
		if constexpr (N >= 4) {
			squared += op.w * op.w;
		}
		return op / sqrt(squared);
	}

	template<typename T>
		requires GLSLNumericType<T> 
	T clamp(T op, T min, T max) {
		return std::clamp(op, min, max);
	}

	template<typename T, uint8_t N>
		requires GLSLFloatType<T> and VecSize<N>
	tc::vec_base<T, N> clamp(tc::vec_base<T, N> op, T min, T max)
	{
		for (int i = 0; i < N; ++i) {
			op[i] = std::clamp(op[i], min, max);
		}
	}
}