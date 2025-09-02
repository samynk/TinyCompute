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

	uint32_t ceil_div(uint32_t a, uint32_t b) {
		return a / b + (a % b != 0);
	}

	void Kernel::dispatch()
	{
		uint32_t workGroupSizeX = ceil_div(m_Width, m_LocalSize.x);
		uint32_t workGroupSizeY = ceil_div(m_Height, m_LocalSize.y);

		const uint32_t totalWorkGroups = workGroupSizeX * workGroupSizeY;
		auto range = std::views::iota(uint32_t{ 0 }, totalWorkGroups);
		uvec3 workGroupSize{ workGroupSizeX, workGroupSizeY, 1 };
		std::for_each(std::execution::par,
			range.begin(), range.end(),
			[&](const uint32_t workgroupID) {
				workGroupID = unflatten(workgroupID, workGroupSize);
				dispatchWorkGroup();
			});
	}

	

	void Kernel::dispatchWorkGroup()
	{
		const uint32_t localThreads = m_LocalSize.x * m_LocalSize.y * m_LocalSize.z;
		auto threadRange = std::views::iota(uint32_t{ 0 }, localThreads);
		std::for_each(std::execution::seq,
			threadRange.begin(), threadRange.end(),
			[&](const uint32_t threadID) {
				localInvocationID = unflatten(threadID, m_LocalSize);
				globalInvocationID.x = workGroupID.x * m_LocalSize.x + localInvocationID.x;
				globalInvocationID.y = workGroupID.y * m_LocalSize.y + localInvocationID.y;
				globalInvocationID.z = workGroupID.z * m_LocalSize.z + localInvocationID.z;
				main();
			});

	}

	void Kernel::main() {
		int n = 0;
		int c = globalInvocationID.x;
		int r = globalInvocationID.y;
		int cellIndex = coordinateToIndex(c, r);
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
