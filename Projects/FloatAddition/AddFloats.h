#pragma once

#include "vec.hpp"
#include "computebackend.hpp"

#include <source_location>
#include <string_view>

struct [[clang::annotate("kernel")]] FloatAdder
{
	static constexpr char fileLocation[] = "floatadder";

	tc::uvec3 local_size{ 256, 1, 1 };
	tc::BufferBinding<float, 0> A;
	tc::BufferBinding<float, 1> B;
	tc::BufferBinding<float, 2> C;

	void main() {
		tc::uint i = tc::gl_GlobalInvocationID.x;
		C[i] = A[i] + B[i];
	}
};