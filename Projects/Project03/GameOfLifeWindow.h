#pragma once
#include "SurfaceRenderer.h"
#include "ComputeShader.h"
#include "GameOfLife.h"
#include "GL/glew.h"

#include <chrono>

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

	void init(SurfaceRenderer& renderer);
	void compute(SurfaceRenderer& renderer);
	unsigned int getTextureID() {
		if (m_pImage2) {
			return m_pImage2->getSSBO_ID();
		}
		else {
			return 0;
		}
	}
	
private:
	GameOfLifeKernel m_GameOfLife;
	tc::BufferResource<tc::R8UI, tc::Dim::D2>* m_pImage1;
	tc::BufferResource<tc::R8UI, tc::Dim::D2>* m_pImage2;

	ConvertKernel m_ConvertKernel;

	uint32_t m_FrameCount{ 0 };
};
