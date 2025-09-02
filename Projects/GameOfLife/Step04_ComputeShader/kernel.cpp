#include "annotations.hpp"
#include <cstddef>

struct [[shader::set(0)]] SquareKernel
{
	// annotated buffers become GLSL SSBOs or UAVs
	[[shader::binding(0)]] float* inData = nullptr;   // readonly
	[[shader::binding(1)]] float* outData = nullptr;   // writeonly
	[[shader::uniform]] std::size_t count = 0;

	// entry function – matches KernelEntry concept
	constexpr void operator()() const
	{
		unsigned idx = sf::gl_GlobalInvocationID.x;
		outData[idx] = inData[idx] * inData[idx];
	}
};