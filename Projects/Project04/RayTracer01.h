#pragma once

#include <iostream>
#include "vec.hpp"
#include "math/arithmetic.hpp"
#include "math/linearalgebra.hpp"
#include "computebackend.hpp"

struct [[clang::annotate("kernel")]] RayTracerKernel
{
	static constexpr char fileLocation[] = "raytracer_v0";

	tc::uvec3 local_size{ 16, 16, 1 };
	// annotated buffers become GLSL SSBOs or UAVs
	tc::ImageBinding<tc::GPUFormat::RGBA32F, tc::Dim::D2, tc::cpu::RGBA8, 1> outData;
	tc::Uniform<float, 0> fieldOfView{1.73};

	// entry function matches KernelEntry concept
	void main()
	{
		tc::uvec2 gId = tc::gl_GlobalInvocationID["xy"_sw];
		tc::ivec2 coordinate = tc::ivec2(gId.x, gId.y);
		tc::ivec2 imgSize = tc::imageSize(outData);

		float aspectRatio = imgSize.x * 1.0f / imgSize.y;
		
		float xcs = (2.0f * (gId.x + 0.5f) / imgSize.x - 1.0f) * aspectRatio * fieldOfView;
		float ycs = (1.0f - 2.0f * (gId.y + 0.5f) / imgSize.y) * fieldOfView;
		tc::vec3 dir = tc::vec3(xcs, ycs, 1.0f);
		tc::vec3 ray = tc::normalize(dir);
		tc::imageStore(outData, coordinate,tc::vec4(ray, 1));
	}
};

struct [[clang::annotate("kernel")]] VisualizeRaysKernel
{
	static constexpr char fileLocation[] = "visualize_rays";
	tc::uvec3 local_size{ 16, 16, 1 };
	tc::ImageBinding<tc::GPUFormat::RGBA32F, tc::Dim::D2, tc::cpu::RGBA8, 0> inData;
	tc::ImageBinding<tc::GPUFormat::RGBA8, tc::Dim::D2, tc::cpu::RGBA8UI, 1> outData;

	void main()
	{
		tc::uvec2 gId = tc::gl_GlobalInvocationID["xy"_sw];
		tc::ivec2 coordinate = tc::ivec2(gId.x, gId.y);

		tc::vec4 ray = tc::imageLoad(inData, coordinate) + tc::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		tc::imageStore(outData,coordinate, ray * 0.5f);
	}
};
