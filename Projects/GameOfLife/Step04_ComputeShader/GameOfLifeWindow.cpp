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

	tc::CPUBackend cpu;
	cpu.useKernel(m_GameOfLife);
	cpu.bindBuffer(m_GameOfLife.inData);
	cpu.bindBuffer(m_GameOfLife.outData);

	cpu.execute(m_GameOfLife, tc::uvec3{ m_GameOfLife.width,m_GameOfLife.height,1 });
	m_GameOfLife._printToConsole();
}

void GameOfLifeWindow::compute(SurfaceRenderer& renderer)
{

	tc::CPUBackend cpu;
	cpu.useKernel(m_GameOfLife);
	cpu.bindBuffer(m_GameOfLife.inData);
	cpu.bindBuffer(m_GameOfLife.outData);
	cpu.bindUniform(m_GameOfLife.width);
	cpu.bindUniform(m_GameOfLife.height);
	cpu.bindUniform(m_GameOfLife.pad);

	cpu.execute(m_GameOfLife, tc::uvec3{ m_GameOfLife.width,m_GameOfLife.width,1 });

	// m_GameOfLife._printToConsole();
	// swap buffer views
	auto oldFrame = m_GameOfLife.inData.getBufferData();
	auto newFrame = m_GameOfLife.outData.getBufferData();

	m_GameOfLife.inData.attach(newFrame);
	m_GameOfLife.outData.attach(oldFrame);
}

void GameOfLifeWindow::setCellIn(int r, int c)
{
	int index = m_GameOfLife.coordinateToIndex(r,c);
	(*m_pDataIn)[index] = 1;
}
