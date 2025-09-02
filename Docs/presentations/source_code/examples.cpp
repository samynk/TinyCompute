#pragma once

#include <iostream>
#include "vec.hpp"
#include "computebackend.hpp"

struct [[clang::annotate("kernel")]] GameOfLifeKernel
{
	static constexpr char fileLocation[] = "gol_v4";

	tc::uvec3 local_size{ 4, 4, 1 };
	tc::BufferBinding<tc::uint, 0, 0> inData;
	tc::BufferBinding<tc::uint, 1, 0> outData;

	tc::Uniform<tc::integer, 1> width{ 16 };
	tc::Uniform<tc::integer, 2> height{ 16 };
	tc::Uniform<tc::integer, 3> pad{ 1 };

	int coordinateToIndex(int cx, int cy) {
		return (cy + pad) * (width+pad) + (cx + pad);
	}

	void main() {
		using namespace tc;
		uint n = 0;
		uvec2 cr = gl_GlobalInvocationID["xy"_sw];
		int cellIndex = coordinateToIndex(cr.x, cr.y);
		uint alive = inData[cellIndex];
		// convolution
		for (int dy = -1; dy <= 1; ++dy)
		{
			for (int dx = -1; dx <= 1; ++dx)
			{
				if (dy || dx) {
					n += inData[coordinateToIndex(cr.x + dx, cr.y + dy)];
				}
			}
		}
		outData[cellIndex] = (n == 3) || (alive && n == 2);
	}
}