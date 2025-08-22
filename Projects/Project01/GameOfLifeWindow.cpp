#include "GameOfLifeWindow.h"
#include "OpenGLBackend.hpp"

GameOfLifeWindow::GameOfLifeWindow(GLuint width, GLuint height)
{

}

GameOfLifeWindow::~GameOfLifeWindow()
{
}

void GameOfLifeWindow::init(const SurfaceRenderer& renderer)
{

	tc::uint w = m_GameOfLife.WIDTH;
	tc::uint h = m_GameOfLife.HEIGHT;
	tc::integer N = w * h;
	m_initDataIn = tc::BufferResource<tc::uint>{ N };
	m_initDataOut = tc::BufferResource<tc::uint>{ N };
	tc::uint offsetX = 7;
	tc::uint offsetY = 7;
	tc::uint pw = 18;
	tc::uint ph = 18;
	for (int y = 0; y < ph; ++y) {
		for (int x = 0; x < pw; ++x) {
			std::size_t idx = (y+offsetY) * w + x+offsetX;
			char c = pattern[y * (pw + 1) + x];
			m_initDataIn[idx] = (c == '1') ? 1 : 0;
			m_initDataOut[idx] = m_initDataIn[idx];
		}
	}

	m_GameOfLife.inData.attach(&m_initDataIn);
	m_GameOfLife.outData.attach(&m_initDataOut);
	m_GameOfLife._printToConsole();
	m_GameOfLife.globalWorkSize.get().x = N;

	GPUBackend gpuBackend;
	gpuBackend.uploadBuffer(m_initDataIn);
	gpuBackend.uploadBuffer(m_initDataOut);

	m_GameOfLife._printToConsole();
	
	gpuBackend.useKernel(m_GameOfLife);
	gpuBackend.bindBuffer(m_GameOfLife.inData);
	gpuBackend.bindBuffer(m_GameOfLife.outData);
	gpuBackend.bindUniform(m_GameOfLife.WIDTH);
	gpuBackend.bindUniform(m_GameOfLife.HEIGHT);
	gpuBackend.execute(m_GameOfLife, m_GameOfLife.globalWorkSize.get());

	gpuBackend.downloadBuffer(m_initDataOut);
	m_GameOfLife._printToConsole();
}

void GameOfLifeWindow::compute(const SurfaceRenderer& renderer)
{
	tc::CPUBackend backend;
	backend.execute(m_GameOfLife, m_GameOfLife.globalWorkSize);
	m_GameOfLife._printToConsole();
	// swap buffer views
	tc::BufferResource<tc::uint>* oldFrame = m_GameOfLife.inData.getBufferData();
	tc::BufferResource<tc::uint>* newFrame = m_GameOfLife.outData.getBufferData();
	m_GameOfLife.inData.attach(newFrame);
	m_GameOfLife.outData.attach(oldFrame);
}