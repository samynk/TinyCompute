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
		return (cy + pad) * (width + pad) + (cx + pad);
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

	void _printToConsole()
	{
		for (int r = 0; r < width; ++r) {
			for (int c = 0; c < height; ++c)
			{
				int idx = coordinateToIndex(c, r);
				switch (outData[idx])
				{
				case 0: std::cout << "\033[1;47m  \033[0m"; break;
				case 1: std::cout << "\033[1;42m  \033[0m"; break;
				}
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}

};

struct [[clang::annotate("kernel")]] ConvertKernel
{
	static constexpr char fileLocation[] = "convert";
	tc::uvec3 local_size{ 16, 16, 1 };

	tc::BufferBinding<tc::uint, 0> inData;
	tc::ImageBinding<tc::InternalFormat::RGBA8, tc::Dim::D2, tc::cpu::RGBA8UI, 1> outData;

	tc::Uniform<tc::integer, 1> width{ 16 };
	tc::Uniform<tc::integer, 2> height{ 16 };
	tc::Uniform<tc::integer, 3> pad{ 1 };
	tc::Uniform<tc::integer, 4> scale{ 8 };

	int coordinateToIndex(int cx, int cy) {
		return (cy + pad) * (width + pad) + (cx + pad);
	}

	void main()
	{
		tc::uvec2 gId = tc::gl_GlobalInvocationID["xy"_sw];
		tc::ivec2 coordinate = tc::ivec2(gId.x, gId.y);

		int cellIndex = coordinateToIndex(coordinate.x / scale, coordinate.y / scale);

		if (cellIndex < (width + pad) * (height + pad))
		{
			if (inData[cellIndex] > 0)
			{
				tc::imageStore(outData, coordinate, tc::vec4(34.0f / 255.0f, 177.0f / 255.0f, 76 / 255.0f, 1.0f));
			}
			else
			{
				tc::imageStore(outData, coordinate, tc::vec4(0, 0, 0, 1.0f));
			}
		}
		else {
			tc::imageStore(outData, coordinate, tc::vec4(1, 0, 0, 1));
		}
	}
};