#include "RayTracerWindow.h"
#include "OpenGLBackend.hpp"
#include "ImageLoader.h"

#include "kernel_intrinsics.hpp"
#include "math/arithmetic.hpp"
#include "images/ImageFormat.hpp"


RayTracerWindow::RayTracerWindow(GLuint width, GLuint height)
	:m_pCameraRaysImage{nullptr}
{

}

RayTracerWindow::~RayTracerWindow()
{
}

void RayTracerWindow::init(SurfaceRenderer& renderer)
{
	tc::ivec2 dim{ renderer.getWidth(),renderer.getHeight()};

	tc::gpu::GPUBackend backend;
	
	m_pCameraRaysImage = new tc::BufferResource<tc::cpu::RGBA8, tc::Dim::D2>{ dim };
	backend.uploadImage<tc::InternalFormat::RGBA32F> (*m_pCameraRaysImage);

	m_RayTracerKernel.outData.attach(m_pCameraRaysImage);
	m_SphereRayTracer.rays.attach(m_pCameraRaysImage);

	m_pTBuffer = std::make_unique<tc::BufferResource<float>>(dim.x * dim.y);
	m_pTBuffer->fill(1000.0f);
	backend.uploadBuffer(*m_pTBuffer);

	m_SphereRayTracer.tBuffer.attach(m_pTBuffer.get());

	m_pSpheres = std::make_unique<SphereBuffer>(4);
	
	(*m_pSpheres)[0] = { tc::vec4{ 0.0,0.75,3.0f, 0.8f}, tc::vec4(1, .5, 0.2, 1) };
	(*m_pSpheres)[1] = { tc::vec4{ 0.7,-0.25,3,0.8f}, tc::vec4(0.1, 1, 0.1, 1) };
	(*m_pSpheres)[2] = { tc::vec4{ 0,-50,48, 60.0 }, tc::vec4(0.1, 0.2, 1, 1)};
	(*m_pSpheres)[3] = { tc::vec4{ -0.7,-0.25,3, 0.8f}, tc::vec4(1, 0.2, 1, 1) };

	m_SphereRayTracer.nrOfSpheres = 4;
	backend.uploadBuffer(*m_pSpheres);
	m_SphereRayTracer.spheres.attach(m_pSpheres.get());
	m_SphereRayTracer.outputTexture.attach(renderer.getRenderBuffer());
}

void RayTracerWindow::compute(SurfaceRenderer& renderer)
{
		tc::gpu::GPUBackend backend;
		//tc::CPUBackend backend;
		
		tc::ivec2 dim{ renderer.getWidth(),renderer.getHeight() };
	
		backend.useKernel(m_RayTracerKernel);
		backend.bindImage(m_RayTracerKernel.outData);
		backend.bindUniform(m_RayTracerKernel.fieldOfView);
		backend.execute(m_RayTracerKernel, tc::uvec3{ dim.x,dim.y,1 });

		backend.useKernel(m_SphereRayTracer);
		backend.uploadBuffer(*m_pTBuffer);
		backend.bindImage(m_SphereRayTracer.rays);
		backend.bindImage(m_SphereRayTracer.outputTexture);
		backend.bindBuffer(m_SphereRayTracer.tBuffer);
		backend.bindBuffer(m_SphereRayTracer.spheres);
		backend.bindUniform(m_SphereRayTracer.nrOfSpheres);
		backend.bindUniform(m_SphereRayTracer.lightPos);
		backend.execute(m_SphereRayTracer, tc::uvec3{ dim.x,dim.y,1 });
}