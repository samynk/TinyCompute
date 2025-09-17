#ifndef TC_RayTracerWindow
#define TC_RayTracerWindow

#include "SurfaceRenderer.hpp"
#include "ComputeShader.hpp"
#include "RayTracerCPU.hpp"
#include "GL/glew.h"

class RayTracerWindow
{
public:
	RayTracerWindow(GLuint width, GLuint height);
	~RayTracerWindow();

	void init(SurfaceRenderer& renderer);
	void compute(SurfaceRenderer& renderer);
	
private:
	void computeRays(SurfaceRenderer& renderer, tc::ivec2 dim);
	void computeScene(SurfaceRenderer& renderer, tc::ivec2 dim);

	RayTracerKernel m_RayTracerKernel;
	SphereRayTracer m_SphereRayTracer;
	
	tc::BufferResource<tc::cpu::RGBA8, tc::Dim::D2>* m_pCameraRaysImage;

	using SphereBuffer = tc::BufferResource<SphereRayTracer::Sphere>;
	std::unique_ptr<SphereBuffer> m_pSpheres;

	using TBuffer = tc::BufferResource<float>;
	std::unique_ptr<TBuffer> m_pTBuffer;
};

#endif
