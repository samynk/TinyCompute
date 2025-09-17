#pragma once
#include "SurfaceRenderer.hpp"
#include "ComputeShader.hpp"
#include "RayTracer02.h"
#include "GL/glew.h"

#include <chrono>

class RayTracerWindow
{
public:
	RayTracerWindow(GLuint width, GLuint height);
	~RayTracerWindow();

	void init(SurfaceRenderer& renderer);
	void compute(SurfaceRenderer& renderer);
	
private:
	RayTracerKernel m_RayTracerKernel;
	SphereRayTracer m_SphereRayTracer;
	tc::BufferResource<tc::cpu::RGBA8, tc::Dim::D2>* m_pCameraRaysImage;
	uint32_t m_FrameCount{ 0 };

	using SphereBuffer = tc::BufferResource<SphereRayTracer::Sphere>;
	std::unique_ptr<SphereBuffer> m_pSpheres;

	using TBuffer = tc::BufferResource<float>;
	std::unique_ptr<TBuffer> m_pTBuffer;
};
