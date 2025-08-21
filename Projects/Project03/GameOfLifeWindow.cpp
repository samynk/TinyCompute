#include "GameOfLifeWindow.h"
#include "OpenGLBackend.h"
#include "ImageLoader.h"

#include "kernel_intrinsics.hpp"
#include "math/arithmetic.hpp"
#include "images/ImageFormat.h"


GameOfLifeWindow::GameOfLifeWindow(GLuint width, GLuint height)
	:m_pImage1{nullptr},m_pImage2{nullptr}
{

}

GameOfLifeWindow::~GameOfLifeWindow()
{
}

void GameOfLifeWindow::init(SurfaceRenderer& renderer)
{
	m_pImage1 =
		tc::assets::loadImage<tc::R8UI>("patterns/methuselah.png");
	m_GameOfLife.inData.attach(m_pImage1);

	tc::ivec2 dim = tc::imageSize(m_GameOfLife.inData);

	m_pImage2 = new tc::BufferResource<tc::R8UI, tc::Dim::D2>{ dim };
	m_GameOfLife.outData.attach(m_pImage2);	

	GPUBackend gpuBackend;
	gpuBackend.uploadImage<tc::GPUFormat::R8UI>(*m_pImage1);
	gpuBackend.uploadImage<tc::GPUFormat::R8UI>(*m_pImage2);

	m_ConvertKernel.outData.attach(renderer.getRenderBuffer());
}

void GameOfLifeWindow::compute(SurfaceRenderer& renderer)
{
	m_FrameCount++;
	if (m_FrameCount < 2) {
		return;
	}
	else {
		GPUBackend gpuBackend;
		// actually do the binding. (corresponds to glBindBufferBase, no-op on cpu, already bound in memory).
		gpuBackend.useKernel(m_GameOfLife);
		gpuBackend.bindImage(m_GameOfLife.inData);
		gpuBackend.bindImage(m_GameOfLife.outData);
		tc::ivec2 dim = tc::imageSize(m_GameOfLife.inData);
		gpuBackend.execute(m_GameOfLife, tc::uvec3{ dim.x,dim.y,1 });

		// m_GameOfLife._printToConsole();
		// swap buffer views
		auto oldFrame = m_GameOfLife.inData.getBufferData();
		auto newFrame = m_GameOfLife.outData.getBufferData();

		m_GameOfLife.inData.attach(newFrame);
		m_GameOfLife.outData.attach(oldFrame);
		
		gpuBackend.useKernel(m_ConvertKernel);
		m_ConvertKernel.inData.attach(newFrame);
		gpuBackend.bindImage(m_ConvertKernel.inData);
		gpuBackend.bindImage(m_ConvertKernel.outData);
		gpuBackend.execute(m_ConvertKernel, tc::uvec3{ dim.x,dim.y,1 });
		m_FrameCount = 0;
	}
}