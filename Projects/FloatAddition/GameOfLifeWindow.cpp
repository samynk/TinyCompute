#include "GameOfLifeWindow.h"
#include "OpenGLBackend.hpp"
#include "kernel_intrinsics.hpp"

#include <random>


GameOfLifeWindow::GameOfLifeWindow(GLuint width, GLuint height)
{

}

GameOfLifeWindow::~GameOfLifeWindow()
{
}

void GameOfLifeWindow::init(SurfaceRenderer& renderer)
{
	using namespace tc;
	
	m_pA.reset(new BufferResource<float>{ N });
	m_pA->randomize(0, 100);
	m_pB.reset(new BufferResource<float>{ N });
	m_pB->randomize(0, 100);
	m_pC.reset(new BufferResource<float>{ N });
	
	m_FloatAdder.A.attach(m_pA.get());
	m_FloatAdder.B.attach(m_pB.get());
	m_FloatAdder.C.attach(m_pC.get());

	using Backend = tc::gpu::GPUBackend;
	Backend compute;
	compute.uploadBuffer(*m_pA.get());
	compute.uploadBuffer(*m_pB.get());
	compute.uploadBuffer(*m_pC.get());

	compute.useKernel(m_FloatAdder);
	compute.bindBuffer(m_FloatAdder.A);
	compute.bindBuffer(m_FloatAdder.B);
	compute.bindBuffer(m_FloatAdder.C);
	compute.execute(m_FloatAdder, tc::uvec3{ N,1,1 });

	compute.downloadBuffer(*m_pC.get());
}

void GameOfLifeWindow::compute(SurfaceRenderer& renderer)
{
}
