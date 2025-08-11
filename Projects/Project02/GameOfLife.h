#pragma once

#include <iostream>
#include "vec.hpp"
#include "computebackend.hpp"

struct [[clang::annotate("kernel")]] GameOfLifeKernel
{
	static constexpr char fileLocation[] = "gameoflife";

	sf::uvec3 local_size{ 16, 1, 1 };
	// annotated buffers become GLSL SSBOs or UAVs
	sf::BindingPoint<sf::uint, 0, 0> inData;
	sf::BindingPoint<sf::uint, 1, 0> outData;

	sf::Uniform<sf::integer, 1> WIDTH{ 32 };
	sf::Uniform<sf::integer, 2> HEIGHT{ 32 };
	sf::Uniform<sf::uvec3, 0> globalWorkSize{ sf::uvec3{WIDTH * HEIGHT,1,1} };

	sf::uint sampleGrid(sf::uvec2 c)
	{
		if (c.x >= 0 && c.y >= 0 && c.x < WIDTH && c.y < HEIGHT)
		{
			return inData[c.y * WIDTH + c.x];
		}
		else {
			// everything dead outside of borders.
			return 0;
		}
	}

	void saveToGrid(sf::uvec2 c, sf::uint value)
	{
		outData[c.y * WIDTH + c.x] = value;
	}
	 
	// entry function matches KernelEntry concept
	void main()
	{
		sf::uvec2 coordinate = sf::gl_GlobalInvocationID["xy"_sw];
		bool alive = sampleGrid(coordinate);
		int sum = -alive;
		for (int ix = -1; ix < 2; ++ix)
		{
			for (int iy = -1; iy < 2; ++iy)
			{
				sum = sum + sampleGrid(sf::uvec2(coordinate.x + ix, coordinate.y + iy));
			}
		}
		if (alive)
		{
			saveToGrid(coordinate,(sum == 2 || sum == 3));
		}
		else
		{
			saveToGrid(coordinate, (sum == 3));
		}
	}

	void _printToConsole()
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
	
};
