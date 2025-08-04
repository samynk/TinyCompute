#pragma once
//#include <iostream>
#include <kernel_intrinsics.hpp>
#include <stdint.h>
namespace sf{
	class uvec3{
		public:
		uvec3(uint32_t x, uint32_t y, uint32_t z)
		:x{x},y{y},z{z}
		{
		}
		uint32_t x,y,z;
	};
}

struct [[clang::annotate("kernel")]] GameOfLifeKernel
{
	sf::uvec3 local_size{ 18,18,1 };
	sf::BindingPoint<uint8_t, 0, 0> inData;
	void main(){
	}
	
	// annotated buffers become GLSL SSBOs or UAVs
	/*sf::BindingPoint<uint8_t, 0, 0> inData;
	sf::BindingPoint<uint8_t, 0, 1> outData;
	sf::Uniform<sf::uint, 0> count;
	sf::Uniform<sf::integer, 1> WIDTH{ 18 };
	sf::Uniform<sf::integer, 2> HEIGHT{ 18 }; */
	
	/*
	uint8_t sample(int x, int y)
	{
		if (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT)
		{
			return inData[y * WIDTH + x];
		}
		else {
			// everything dead outside of borders.
			return 0;
		}
	}
	*/
	 
	// entry function – matches KernelEntry concept
	/*
	void operator()()
	{
		unsigned idx = sf::gl_GlobalInvocationID.x;
		int x = idx % WIDTH;
		int y = idx / HEIGHT;
		   
		//sf::vec3* pVec = new sf::vec3{ 1,2,3 };

 
		bool alive = inData[idx];
		int sum = -alive;
		for (int ix = -1; ix < 2; ++ix)
		{
			for (int iy = -1; iy < 2; ++iy)
			{
				sum += sample(x + ix, y + iy);
			}
		}
		if (alive)
		{
			outData[idx] = (sum == 2 || sum == 3);
		}
		else
		{
			outData[idx] = (sum == 3);
		}
	}
	*/

	/*
	void printToConsole()
	{
		for (int y = 0; y < HEIGHT; ++y) {
			for (int x = 0; x < WIDTH; ++x)
			{
				std::size_t idx = y * WIDTH + x;
				switch (outData[idx])
				{
				case 0: std::cout << "\033[1;47m  \033[0m"; break;
				case 1: std::cout << "\033[1;42m  \033[0m"; break;
				}
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}
	*/
};
