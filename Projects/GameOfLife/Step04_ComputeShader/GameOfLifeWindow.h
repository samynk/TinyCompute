#pragma once
#include "SurfaceRenderer.hpp"
#include "ComputeShader.hpp"
#include "GameOfLife.h"
#include "GL/glew.h"

class GameOfLifeWindow
{
public:
	GameOfLifeWindow(GLuint width, GLuint height);
	~GameOfLifeWindow();

	void init(SurfaceRenderer& renderer);
	void compute(SurfaceRenderer& renderer);
	void setCellIn(int r, int c);
	
private:
	GameOfLifeKernel m_GameOfLife;
	std::unique_ptr<tc::BufferResource<tc::uint>> m_pDataIn;
	std::unique_ptr<tc::BufferResource<tc::uint>> m_pDataOut;

	uint32_t m_FrameCount{ 0 };
};
