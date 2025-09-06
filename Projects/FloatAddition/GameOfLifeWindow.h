#pragma once
#include "SurfaceRenderer.hpp"
#include "ComputeShader.hpp"
#include "GL/glew.h"
#include "AddFloats.h"

class GameOfLifeWindow
{
public:
	GameOfLifeWindow(GLuint width, GLuint height);
	~GameOfLifeWindow();

	void init(SurfaceRenderer& renderer);
	void compute(SurfaceRenderer& renderer);
private:
	FloatAdder m_FloatAdder;
	std::unique_ptr<tc::BufferResource<float>> m_pA;
	std::unique_ptr<tc::BufferResource<float>> m_pB;
	std::unique_ptr<tc::BufferResource<float>> m_pC;

	static constexpr tc::uint N = 100000;
};
