#include <iostream>
#include <vector>
#include "vec.hpp"
#include "math/arithmetic.hpp"
#include "annotations.hpp"
#include "kernel_intrinsics.hpp"
#include "ShaderBuffer.h"
#include "computebackend.hpp"

#include "GameOfLife.h"

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


int main(void)
{
	GameOfLifeKernel kern{};
	sf::uint w = kern.WIDTH;
	sf::uint h = kern.HEIGHT;
	sf::uint N = w*h;
	sf::BufferResource<uint8_t> initDataIn{ N };
	sf::BufferResource<uint8_t> initDataOut{ N };
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			std::size_t idx = y * w + x;
			char c = pattern[y * (w + 1) + x];
			initDataIn[idx] = (c == '1') ? 1 : 0;
			initDataOut[idx] = initDataIn[idx];
		}
	}

	kern.inData.attach(&initDataIn);
	kern.outData.attach(&initDataOut);
	kern.printToConsole();
	kern.count = N;

	sf::CPUBackend backend;
	// actually do the binding. (corresponds to glBindBufferBase, no-op on cpu, already bound in memory).
	// backend.bindBuffer(kern.inData);
	// backend.bindBuffer(kern.outData);
	for (int frame = 0; frame < 4; ++frame)
	{
		backend.execute(kern, N);
		kern.printToConsole();
		// swap buffer views
		sf::BufferResource<uint8_t>* oldFrame = kern.inData.getBufferData();
		sf::BufferResource<uint8_t>* newFrame = kern.outData.getBufferData();
		kern.inData.attach(newFrame);
		kern.outData.attach(oldFrame);
	}
}



