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
			k.set0(8, 7);
			k.set0(7, 8);
			k.set0(8, 8);
			k.set0(8, 9);
			k.set0(9, 9);
			
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
			if (m_Delay > 500) {
				k.dispatch();
				k.swapBuffers();
				m_Delay = 0;
			}
			m_Delay++;
			return true;
		}
	private:
		GameOfLife::Kernel k{ 16,16,1 };
		int32_t m_Scale{ 20 };
		int32_t m_Delay{ 0 };
	};
}
int main()
{
	using namespace GameOfLife;
	GameOfLifeWindow demo;
	if (demo.Construct(16*20, 16*20, 1, 1))
	{
		demo.Start();
	}
	return 0;

}