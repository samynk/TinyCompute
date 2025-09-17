#pragma once

#include <iostream>
#include "vec.hpp"
#include "math/arithmetic.hpp"
#include "math/linearalgebra.hpp"
#include "computebackend.hpp"

struct [[clang::annotate("kernel")]] RayTracerKernel
{
	static constexpr char fileLocation[] = "raytracer_v2";

	tc::uvec3 local_size{ 16, 16, 1 };
	tc::ImageBinding<tc::InternalFormat::RGBA32F, tc::Dim::D2, tc::cpu::RGBA8, 1> outData;
	tc::Uniform<float, 0> fieldOfView{ 1.2 };

	struct Sphere {
		tc::vec4 locr;
		tc::vec4 color;
	};

	tc::BufferBinding<Sphere, 0> spheres;


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
		tc::imageStore(outData, coordinate, tc::vec4(ray, 1));
	}
};

struct [[clang::annotate("kernel")]] SphereRayTracer
{
	static constexpr char fileLocation[] = "sphere_raytracer";
	tc::uvec3 local_size{ 16,16,1 };
	tc::ImageBinding<tc::InternalFormat::RGBA32F, tc::Dim::D2, tc::cpu::RGBA8, 0> rays;
	tc::BufferBinding<float, 1> tBuffer;

	struct Sphere {
		tc::vec4 locR;
		tc::vec4 color;
	};
	tc::BufferBinding<Sphere, 2> spheres;
	tc::Uniform<int, 0> nrOfSpheres;

	tc::ImageBinding<tc::InternalFormat::RGBA8, tc::Dim::D2, tc::cpu::RGBA8UI, 1> outputTexture;
	tc::Uniform<tc::vec3, 1> lightPos{ tc::vec3{-0.25,3,0.2} };

	void main()
	{
		using namespace tc;
		uvec2 gid = gl_GlobalInvocationID["xy"_sw];
		ivec2 coordinate = ivec2(gid.x, gid.y);
		vec3 rayOrigin = vec3(0, 0, 0);
		vec3 rayDirection = imageLoad(rays, coordinate)["xyz"_sw];

		ivec2 imgSize = imageSize(rays);
		int index = imgSize.x * coordinate.y + coordinate.x;

		bool write = false;
		vec4 color = vec4(0.05, 0.05, 0.05, 1);
		for (int si = 0; si < nrOfSpheres; ++si)
		{
			vec3 sphereLoc = spheres[si].locR["xyz"_sw];
			float r = spheres[si].locR.w;

			vec3 raySphereDiff = sphereLoc - rayOrigin; // take origin of ray (camera into account)
			float L2 = dot(raySphereDiff, raySphereDiff);
			float tca = dot(raySphereDiff, rayDirection);
			float od2 = L2 - tca * tca;

			float r2 = r * r;
			if (od2 < r2) {
				float thc = sqrt(r2 - od2);
				float t0 = tca - thc;
				if (t0 > 0 && t0 <= tBuffer[index]) {
					vec3 currentPos = t0 * rayDirection + rayOrigin;
					vec3 currentNormal = (currentPos - sphereLoc) / r;
					vec3 lp = lightPos;
					vec3 lightVec = normalize(lp - currentPos);
					float Id = clamp(dot(lightVec, currentNormal), 0.2f, 1.0f);

					color = Id * spheres[si].color;
					tBuffer[index] = t0;
				}

			}
		}

		// Write the value to the output texture
		imageStore(outputTexture, coordinate, color);
	}
};

struct [[clang::annotate("kernel")]] VisualizeRaysKernel
{
	static constexpr char fileLocation[] = "visualize_rays";
	tc::uvec3 local_size{ 16, 16, 1 };
	tc::ImageBinding<tc::InternalFormat::RGBA32F, tc::Dim::D2, tc::cpu::RGBA8, 0> inData;
	tc::ImageBinding<tc::InternalFormat::RGBA8, tc::Dim::D2, tc::cpu::RGBA8UI, 1> outData;

	void main()
	{
		tc::uvec2 gId = tc::gl_GlobalInvocationID["xy"_sw];
		tc::ivec2 coordinate = tc::ivec2(gId.x, gId.y);

		tc::vec4 ray = tc::imageLoad(inData, coordinate) + tc::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		tc::imageStore(outData, coordinate, ray * 0.5f);
	}
};
