#include "Kernel.hpp"
#include <execution>
#include <algorithm>
#include <ranges>
#include <span>
#include <numeric>

namespace GameOfLife {

	Kernel::Kernel(size_t w, size_t h, size_t pad) :
		m_Width{ w },
		m_PaddedWidth{ w + 2 * pad },
		m_Height{ h },
		m_PaddedHeight{ h + 2 * pad },
		m_Pad{ pad },
		m_State0(m_PaddedWidth* m_PaddedHeight, 0u),
		m_State1(m_State0.size(), 0u)
	{

	}

	void Kernel::dispatch()
	{
		const uint64_t totalWork = m_Width * m_Height;
		auto range = std::views::iota(uint64_t{ 0 }, totalWork);
		uvec3 globalWorkSize{ m_Width,m_Height,1 };
		std::for_each(std::execution::par,
			range.begin(), range.end(),
			[&](const uint64_t xi) {
				globalInvocationID = unflatten(xi, globalWorkSize);
				main();
			});
	}

	void Kernel::main() {
		int n = 0;
		int c = globalInvocationID.x;
		int r = globalInvocationID.y;
		int cellIndex = coordinateToIndex(c,r);
		int alive = m_State0[cellIndex];
		// convolution
		for (index dy = -1; dy <= 1; ++dy)
		{
			for (index dx = -1; dx <= 1; ++dx)
			{

				if (dy || dx) {
					n += m_State0[coordinateToIndex(c + dx, r + dy)];
				}
			}
		}
		m_State1[cellIndex] = (n == 3) || (alive && n == 2);
	}
}

