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
	std::unique_ptr<tc::BufferResource<tc::cpu::R8UI,tc::Dim::D2>> m_pDataIn;
	std::unique_ptr<tc::BufferResource<tc::cpu::R8UI,tc::Dim::D2>> m_pDataOut;

	uint32_t m_FrameCount{ 0 };

	ConvertKernel m_ConvertKernel;
};
