#pragma once
#include "SurfaceRenderer.h"
#include "ComputeShader.h"
#include "GameOfLife.h"
#include "GL/glew.h"

constexpr std::string_view pattern =
R"(000000000000000000
000000000000000000
000000000000000000
000011100011100000
000000000000000000
001000010100001000
001000010100001000
001000010100001000
000011100011100000
000000000000000000
000011100011100000
001000010100001000
001000010100001000
001000010100001000
000000000000000000
000011100011100000
000000000000000000
000000000000000000
)";



class GameOfLifeWindow
{
public:
	GameOfLifeWindow(GLuint width, GLuint height);
	~GameOfLifeWindow();

	void init(const SurfaceRenderer& renderer);
	void compute(const SurfaceRenderer& renderer);

	unsigned int getTextureID() {
		return 0;
	}
private:
	GameOfLifeKernel m_GameOfLife;

	tc::BufferResource<tc::uint> m_initDataIn;
	tc::BufferResource<tc::uint> m_initDataOut;
};
