#include "Kernel.hpp"
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

	void Kernel::execute()
	{
		for (index r = 0; r < m_Height; ++r)
		{
			for (index c = 0; c < m_Width; ++c)
			{
				int n = 0;
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
	}
}