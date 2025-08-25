#pragma once
#include "SurfaceRenderer.hpp"
#include "ComputeShader.hpp"
#include "RayTracer01.h"
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
	VisualizeRaysKernel m_VisualizeRaysKernel;
	tc::BufferResource<tc::cpu::RGBA8, tc::Dim::D2>* m_pCameraRaysImage;
	uint32_t m_FrameCount{ 0 };
};
