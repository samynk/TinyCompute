#include <array>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "Kernel.hpp"

namespace GameOfLife
{
	class GameOfLifeWindow : public olc::PixelGameEngine
	{
	public:
		GameOfLifeWindow() {
			
			sAppName = "GameOfLife _ Step 0";
		}
	
		bool OnUserCreate() override
		{
			size_t hw = k.getWidth()/2;
			size_t hh = k.getHeight() / 2;
			k.set0(hw + 1, hh);
			k.set0(hw, hh + 1);
			k.set0(hw + 1, hh + 1);
			k.set0(hw + 1, hh + 2);
			k.set0(hw + 2, hh + 2);
			
			return true;
		}

		bool OnUserUpdate(float fElapsedTime) override
		{
			// called once per frame
			
			
			for (int32_t x = 0; x < k.getWidth(); x++)
			{
				for (int32_t y = 0; y < k.getHeight(); y++)
				{
					uint32_t cell = k.get1(x, y);
					olc::Pixel color = cell ? olc::GREEN : olc::BLACK;
					FillRect(x * m_Scale, y * m_Scale, m_Scale, m_Scale, color);
				}
			}
			if (m_Delay > 10) {
				k.dispatch();
				k.swapBuffers();
				m_Delay = 0;
			}
			m_Delay++;
			return true;
		}
	private:
		GameOfLife::Kernel k{ 32,32,1 };
		int32_t m_Scale{ 20 };
		int32_t m_Delay{ 0 };
	};
}
int main()
{
	using namespace GameOfLife;
	GameOfLifeWindow demo;
	if (demo.Construct(32*20, 32*20, 1, 1))
	{
		demo.Start();
	}
	return 0;

}