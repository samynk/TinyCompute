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
	integer w = renderer.getWidth() / m_ConvertKernel.scale + 2 * m_GameOfLife.pad;
	integer h = renderer.getHeight() / m_ConvertKernel.scale + 2 * m_GameOfLife.pad;

	tc::ivec2 dim{ w,h };
	m_pDataIn.reset(new BufferResource<tc::cpu::R8UI, tc::Dim::D2>{ dim });
	m_pDataOut.reset(new BufferResource<tc::cpu::R8UI, tc::Dim::D2>{ dim });

	m_GameOfLife.inData.attach(m_pDataIn.get());
	m_GameOfLife.outData.attach(m_pDataOut.get());

	ivec2 offset{ w / 2 + m_GameOfLife.pad, h / 2 + m_GameOfLife.pad };
	tc::imageStore(m_GameOfLife.inData, ivec2{ 1, 0 } + offset, 1);
	tc::imageStore(m_GameOfLife.inData, ivec2{ 0, 1 } + offset, 1);
	tc::imageStore(m_GameOfLife.inData, ivec2{ 1, 1 } + offset, 1);
	tc::imageStore(m_GameOfLife.inData, ivec2{ 1, 2 } + offset, 1);
	tc::imageStore(m_GameOfLife.inData, ivec2{ 2, 2 } + offset, 1);

	uint cell1 = tc::imageLoad(m_GameOfLife.inData, ivec2{ 1, 0 } + offset).x;
	uint cell2 = tc::imageLoad(m_GameOfLife.inData, ivec2{ 0,1 } + offset).x;
	uint cell3 = tc::imageLoad(m_GameOfLife.inData, ivec2{ 1, 1 } + offset).x;
	uint cell4 = tc::imageLoad(m_GameOfLife.inData, ivec2{ 1, 2 } + offset).x;
	uint cell5 = tc::imageLoad(m_GameOfLife.inData, ivec2{ 2,2 } + offset).x;

	uint testCell = tc::imageLoad(m_GameOfLife.inData, ivec2{ 3,3 } + offset).x;

	tc::gpu::GPUBackend gpu;
	gpu.uploadImage<tc::InternalFormat::R8UI>(*m_pDataIn.get());
	gpu.uploadImage<tc::InternalFormat::R8UI>(*m_pDataOut.get());

	m_ConvertKernel.inData.attach(m_pDataOut.get());
	m_ConvertKernel.outData.attach(renderer.getRenderBuffer());
}

void GameOfLifeWindow::compute(SurfaceRenderer& renderer)
{
	//using Backend = tc::gpu::GPUBackend;
	using Backend = tc::CPUBackend;
	Backend b;
	b.useKernel(m_GameOfLife);
	b.bindImage(m_GameOfLife.inData);
	b.bindImage(m_GameOfLife.outData);
	b.bindUniform(m_GameOfLife.pad);
	tc::ivec2 dim = tc::imageSize(m_GameOfLife.inData);
	b.execute(m_GameOfLife, tc::uvec3{ dim.x - 2 * m_GameOfLife.pad, dim.y - 2 * m_GameOfLife.pad, 1 });

	auto oldFrame = m_GameOfLife.inData.getBufferData();
	auto newFrame = m_GameOfLife.outData.getBufferData();

	m_GameOfLife.inData.attach(newFrame);
	m_GameOfLife.outData.attach(oldFrame);

	b.useKernel(m_ConvertKernel);
	m_ConvertKernel.inData.attach(newFrame);
	b.bindImage(m_ConvertKernel.inData);
	b.bindImage(m_ConvertKernel.outData);
	b.bindUniform(m_ConvertKernel.pad);
	b.bindUniform(m_ConvertKernel.scale);
	int scale = m_ConvertKernel.scale;
	b.execute(m_ConvertKernel, tc::uvec3{ renderer.getWidth(),renderer.getHeight(),1 });
}
