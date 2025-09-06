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
	m_VisualizeRaysKernel.inData.attach(m_pCameraRaysImage);
	m_VisualizeRaysKernel.outData.attach(renderer.getRenderBuffer());
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

		backend.useKernel(m_VisualizeRaysKernel);
		backend.bindImage(m_VisualizeRaysKernel.inData);
		backend.bindImage(m_VisualizeRaysKernel.outData);
		backend.execute(m_VisualizeRaysKernel, tc::uvec3{ dim.x,dim.y,1 });
}