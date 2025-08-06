#include "GameOfLifeWindow.h"
#include "OpenGLBackend.h"

GameOfLifeWindow::GameOfLifeWindow(GLuint width, GLuint height)
{

}

GameOfLifeWindow::~GameOfLifeWindow()
{
}

void GameOfLifeWindow::init(const SurfaceRenderer& renderer)
{
	sf::uint w = m_GameOfLife.WIDTH;
	sf::uint h = m_GameOfLife.HEIGHT;
	sf::uint N = w * h;
	m_initDataIn = sf::BufferResource<uint8_t>{ N };
	m_initDataOut = sf::BufferResource<uint8_t>{ N };
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			std::size_t idx = y * w + x;
			char c = pattern[y * (w + 1) + x];
			m_initDataIn[idx] = (c == '1') ? 1 : 0;
			m_initDataOut[idx] = m_initDataIn[idx];
		}
	}

	m_GameOfLife.inData.attach(&m_initDataIn);
	m_GameOfLife.outData.attach(&m_initDataOut);
	m_GameOfLife._printToConsole();
	m_GameOfLife.count = N;

	GPUBackend gpuBackend;
	gpuBackend.execute(m_GameOfLife,m_GameOfLife.count);
}

void GameOfLifeWindow::compute(const SurfaceRenderer& renderer)
{
	sf::CPUBackend backend;
	// actually do the binding. (corresponds to glBindBufferBase, no-op on cpu, already bound in memory).
	// backend.bindBuffer(kern.inData);
	// backend.bindBuffer(kern.outData);
	for (int frame = 0; frame < 4; ++frame)
	{
		backend.execute(m_GameOfLife, m_GameOfLife.count);
		m_GameOfLife._printToConsole();
		// swap buffer views
		sf::BufferResource<uint8_t>* oldFrame = m_GameOfLife.inData.getBufferData();
		sf::BufferResource<uint8_t>* newFrame = m_GameOfLife.outData.getBufferData();
		m_GameOfLife.inData.attach(newFrame);
		m_GameOfLife.outData.attach(oldFrame);
	}
}