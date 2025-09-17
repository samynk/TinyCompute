#pragma once

#include <iostream>
#include "vec.hpp"
#include "math/arithmetic.hpp"
#include "computebackend.hpp"

struct [[clang::annotate("kernel")]] GameOfLifeKernel
{
	static constexpr char fileLocation[] = "gol_v4";
	tc::uvec3 local_size{ 4, 4, 1 };
	tc::ImageBinding<tc::InternalFormat::R8UI, tc::Dim::D2, tc::cpu::R8UI, 0> inData;
	tc::ImageBinding<tc::InternalFormat::R8UI, tc::Dim::D2, tc::cpu::R8UI, 1> outData;

	tc::Uniform<tc::integer, 1> pad{ 1 };

	std::array<tc::ivec2, 8> kernelIndices {
		tc::ivec2{-1,-1}, tc::ivec2{0,-1},tc::ivec2{1,-1},
		tc::ivec2{-1, 0},                 tc::ivec2{1, 0},
		tc::ivec2{-1, 1}, tc::ivec2{0, 1},tc::ivec2{1, 1}
	};

	void main() {
		using namespace tc;
		uint n = 0;
		uvec2 gId = tc::gl_GlobalInvocationID["xy"_sw];
		ivec2 coordinate = ivec2(gId.x+pad, gId.y+pad);

		bool alive = imageLoad(inData, coordinate).x;
		for (int ki = 0; ki < kernelIndices.size(); ++ki) {
			n += imageLoad(inData, coordinate + kernelIndices[ki]).x;
		}

		bool newState = (n == 3) || (alive && n == 2);
		imageStore(outData, coordinate, uvec4(newState));
	}
};

struct [[clang::annotate("kernel")]] ConvertKernel
{
	static constexpr char fileLocation[] = "convert";
	tc::uvec3 local_size{ 16, 16, 1 };

	tc::ImageBinding<tc::InternalFormat::R8UI, tc::Dim::D2, tc::cpu::R8UI, 0> inData;
	tc::ImageBinding<tc::InternalFormat::RGBA8, tc::Dim::D2, tc::cpu::RGBA8UI, 1> outData;

	std::array<tc::vec4, 3> colors{
		tc::vec4(0    , 0  , 0  , 1.0),
		tc::vec4(0.133, 0.7, 0.3, 1.0),
		tc::vec4(1    , 0  , 0  , 1.0)
	};

	tc::Uniform<tc::integer, 1> pad{ 1 };
	tc::Uniform<tc::integer, 2> scale{ 2 };

	void main()
	{
		tc::uvec2 gId = tc::gl_GlobalInvocationID["xy"_sw];
		tc::ivec2 rc = tc::ivec2(gId.x , gId.y );
		tc::ivec2 coordinate = rc / scale + tc::ivec2{ pad,pad };

		tc::ivec2 dim = imageSize(inData);

		if (coordinate.x < dim.x && coordinate.y < dim.y)
		{
			tc::uint alive = imageLoad(inData, coordinate).x;
			tc::imageStore(outData, rc, colors[alive]);
		}
		else {
			tc::imageStore(outData, rc, colors[2] );
		}
	}
};