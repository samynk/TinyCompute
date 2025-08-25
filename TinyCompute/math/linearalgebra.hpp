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
}