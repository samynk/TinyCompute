#include "GameOfLifeWindow.h"
#include "OpenGLBackend.hpp"
#include "ImageLoader.h"

#include "kernel_intrinsics.hpp"
#include "math/arithmetic.hpp"
#include "images/ImageFormat.hpp"


GameOfLifeWindow::GameOfLifeWindow(GLuint width, GLuint height)
{

}

GameOfLifeWindow::~GameOfLifeWindow()
{
}

void GameOfLifeWindow::init(SurfaceRenderer& renderer)
{
	using namespace tc;
	integer w = m_GameOfLife.width + 2 * m_GameOfLife.pad;
	integer h = m_GameOfLife.height + 2 * m_GameOfLife.pad;
	integer N = w * h;
	m_pDataIn.reset(new BufferResource<uint>{ N });
	m_pDataOut.reset(new BufferResource<uint>{ N });
	setCellIn(8, 7);
	setCellIn(7, 8);
	setCellIn(8, 8);
	setCellIn(8, 9);
	setCellIn(9, 9);

	m_GameOfLife.inData.attach(m_pDataIn.get());
	m_GameOfLife.outData.attach(m_pDataOut.get());

	m_GameOfLife._printToConsole();

	tc::gpu::GPUBackend gpu;
	gpu.uploadBuffer(*m_pDataIn.get());
	gpu.uploadBuffer(*m_pDataOut.get());

	m_ConvertKernel.inData.attach(m_pDataOut.get());
	m_ConvertKernel.outData.attach(renderer.getRenderBuffer());
}

void GameOfLifeWindow::compute(SurfaceRenderer& renderer)
{
	using Backend = tc::gpu::GPUBackend;
	Backend b;
	b.useKernel(m_GameOfLife);
	b.bindBuffer(m_GameOfLife.inData);
	b.bindBuffer(m_GameOfLife.outData);
	b.bindUniform(m_GameOfLife.width);
	b.bindUniform(m_GameOfLife.height);
	b.bindUniform(m_GameOfLife.pad);
	b.execute(m_GameOfLife, tc::uvec3{ m_GameOfLife.width,m_GameOfLife.width,1 });

	b.useKernel(m_ConvertKernel);
	b.bindBuffer(m_ConvertKernel.inData);
	b.bindImage(m_ConvertKernel.outData);
	b.bindUniform(m_ConvertKernel.width);
	b.bindUniform(m_ConvertKernel.height);
	b.bindUniform(m_ConvertKernel.pad);
	b.bindUniform(m_ConvertKernel.scale);
	int scale = m_ConvertKernel.scale;
	b.execute(m_ConvertKernel, tc::uvec3{ m_GameOfLife.width*scale,m_GameOfLife.width*scale,1 });

	using std::swap;
	swap(m_GameOfLife.inData, m_GameOfLife.outData);
}

void GameOfLifeWindow::setCellIn(int r, int c)
{
	int index = m_GameOfLife.coordinateToIndex(r,c);
	(*m_pDataIn)[index] = 1;
}
