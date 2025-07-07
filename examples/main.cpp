#include <iostream>
#include <vector>
#include "vec.hpp"
#include "annotations.hpp"
#include "kernel_intrinsics.hpp"
#include "ShaderBuffer.h"

consteval void _unroll_check() {
	constexpr sf::vec4 v(1, sf::vec2{ 2,3 }, 0);
}

// ──────────────────────────────────────────────────────────────
// 2.  Example kernel class  (square each element)
// ──────────────────────────────────────────────────────────────
struct SquareKernel
{
	// annotated buffers become GLSL SSBOs or UAVs
	ShaderBuffer<float, 1'000, 0, 0> inData;
	ShaderBuffer<float, 1'000, 0, 1> outData;

	std::size_t count = 0;

	// entry function – matches KernelEntry concept
	void operator()() 
	{
		unsigned idx = sf::gl_GlobalInvocationID.x;
		outData[idx] = inData[idx] * inData[idx];
	}
};


int main(void)
{
	std::cout << "Swizzle forge demo" << std::endl;

	sf::vec3 v1(1, 2, 3);
	sf::vec3 v2(3, 4, 5);

	// operator overloading
	sf::vec3 result = v1 + v2;
	std::cout << "Result:" << result << std::endl;

	// swizzle right value
	sf::vec2 resultXZ = result["xz"_sw];
	std::cout << "Result:" << resultXZ << std::endl;

	// swizzle left value
	v1["x"_sw] = 3.0;
	std::cout << "v1:" << v1 << std::endl;

	sf::vec4 v3(2, 3, 4, 5);
	// swizzle right value, swizzle left value
	v3["yw"_sw] = v2["xz"_sw];
	std::cout << "Result:" << v3 << std::endl;


	sf::vec4 v4(2, sf::vec2(1, 3), 3);
	std::cout << "Result:" << v4 << std::endl;

	_unroll_check();

	const std::size_t N = 1'000'000;
	SquareKernel kern;
	for (std::size_t i = 0; i < N; ++i) {
		kern.inData[i] = sqrtf(static_cast<float>(i));
	}
	kern.count = N;

	sf::dispatch_cpu(kern, N);         // multithreaded CPU run

	std::cout << kern.outData[10] << '\n';   // prints 100
}



