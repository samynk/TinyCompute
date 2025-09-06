#pragma once

#include <iostream>
#include "vec.hpp"
#include "computebackend.hpp"

struct [[clang::annotate("kernel")]] GameOfLifeKernel
{
	static constexpr char fileLocation[] = "gameoflife_v3";

	tc::uvec3 local_size{ 16, 16, 1 };
	// annotated buffers become GLSL SSBOs or UAVs

	tc::ImageBinding<tc::InternalFormat::R8UI, tc::Dim::D2, tc::cpu::R8UI, 0> inData;
	tc::ImageBinding<tc::InternalFormat::R8UI, tc::Dim::D2, tc::cpu::R8UI, 1> outData;
	//tc::Uniform<tc::uvec3, 0> globalWorkSize{ tc::uvec3{WIDTH,HEIGHT,1} };

	tc::uint sampleGrid(tc::ivec2 c, tc::ivec2 dim)
	{
		return tc::imageLoad(inData, c).x > 0 ? 1 : 0;
	}

	void saveToGrid(tc::ivec2 c, tc::uint value)
	{
		tc::imageStore(outData, c, tc::uvec4(value, 0, 0, 1));
	}

	// entry function matches KernelEntry concept
	void main()
	{
		tc::uvec2 gId = tc::gl_GlobalInvocationID["xy"_sw];
		tc::ivec2 coordinate = tc::ivec2(gId.x, gId.y);
		tc::ivec2 imgSize = tc::imageSize(inData);
		bool alive = sampleGrid(coordinate, imgSize);
		int sum = -alive;

		for (int ix = -1; ix < 2; ++ix)
		{
			for (int iy = -1; iy < 2; ++iy)
			{
				sum = sum + sampleGrid(tc::ivec2(coordinate.x + ix, coordinate.y + iy), imgSize);
			}
		}
		if (alive)
		{
			saveToGrid(coordinate, (sum == 2 || sum == 3));
		}
		else
		{
			saveToGrid(coordinate, (sum == 3));
		}
	}
};

struct [[clang::annotate("kernel")]] ConvertKernel
{
	static constexpr char fileLocation[] = "convert";
	tc::uvec3 local_size{ 16, 16, 1 };

	tc::ImageBinding<tc::InternalFormat::R8UI, tc::Dim::D2, tc::cpu::R8UI, 0> inData;
	tc::ImageBinding<tc::InternalFormat::RGBA8, tc::Dim::D2, tc::cpu::RGBA8UI, 1> outData;

	void main()
	{
		tc::uvec2 gId = tc::gl_GlobalInvocationID["xy"_sw];
		tc::ivec2 coordinate = tc::ivec2(gId.x, gId.y);

		if (tc::imageLoad(inData, coordinate).x > 0)
		{
			tc::imageStore(outData, coordinate, tc::vec4(34.0f / 255.0f, 177.0f / 255.0f, 76 / 255.0f, 1.0f));
		}
		else
		{
			tc::imageStore(outData, coordinate, tc::vec4(0, 0, 0, 1.0f));
		}
	}
};
