#pragma once

#include <iostream>
#include "vec.hpp"
#include "computebackend.hpp"

struct [[clang::annotate("kernel")]] GameOfLifeKernel
{
	static constexpr char fileLocation[] = "gameoflife";
	
	tc::uvec3 local_size{ 16, 16, 1 };
	// annotated buffers become GLSL SSBOs or UAVs
	tc::BufferBinding<tc::uint, 0, 0> inData;
	tc::BufferBinding<tc::uint, 1, 0> outData;
	//tc::ImageBinding<tc::GPUFormat::RGBA8, tc::Dim::D2, tc::RGBA8, 1> m_Image;

	tc::Uniform<tc::integer, 1> WIDTH{ 32 };
	tc::Uniform<tc::integer, 2> HEIGHT{ 32 };
	tc::Uniform<tc::uvec3, 0> globalWorkSize{ tc::uvec3{WIDTH,HEIGHT,1} };

	tc::uint sampleGrid(tc::uvec2 c)
	{
		if (c.x >= 0 && c.y >= 0 && c.x < WIDTH && c.y < HEIGHT)
		{
			return inData[c.y * WIDTH + c.x];
		}
		else {
			// everything dead outside of borders.
			return 0;
		}
	}

	void saveToGrid(tc::uvec2 c, tc::uint value)
	{
		outData[c.y * WIDTH + c.x] = value;
	}
	 
	// entry function matches KernelEntry concept
	void main()
	{
		tc::uvec2 coordinate = tc::gl_GlobalInvocationID["xy"_sw];
		bool alive = sampleGrid(coordinate);
		int sum = -alive;
		for (int ix = -1; ix < 2; ++ix)
		{
			for (int iy = -1; iy < 2; ++iy)
			{
				sum = sum + sampleGrid(tc::uvec2(coordinate.x + ix, coordinate.y + iy));
			}
		}
		if (alive)
		{
			saveToGrid(coordinate,(sum == 2 || sum == 3));
		}
		else
		{
			saveToGrid(coordinate, (sum == 3));
		}
	}

	void _printToConsole()
	{
		for (int y = 0; y < HEIGHT; ++y) {
			for (int x = 0; x < WIDTH; ++x)
			{
				std::size_t idx = y * WIDTH + x;
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
