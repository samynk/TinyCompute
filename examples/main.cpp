#include <iostream>
#include <vector>
#include "vec.hpp"
#include "math/arithmetic.hpp"
#include "annotations.hpp"
#include "kernel_intrinsics.hpp"
#include "ShaderBuffer.h"
#include "computebackend.hpp"

consteval void _unroll_check() {
	constexpr sf::vec4 v(1, sf::vec2{ 2,3 }, 0);
}

// ──────────────────────────────────────────────────────────────
// 2.  Example kernel class  (square each element)
// ──────────────────────────────────────────────────────────────
const std::size_t N = 1'000;
struct SquareKernel
{
	sf::uvec3 local_size{ 16,1,1 };
	// annotated buffers become GLSL SSBOs or UAVs
	ShaderBuffer<float, N, 0, 0> inData;
	ShaderBuffer<float, N, 0, 1> outData;

	std::size_t count = 0;

	void setup_cpu()
	{
		for (std::size_t i = 0; i < N; ++i)
		{
			inData[i] = sqrtf(static_cast<float>(i));
		}
	}

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

	//// swizzle right value
	sf::vec2 resultXZ = result["xz"_sw];
	std::cout << "Result:" << resultXZ << std::endl;

	//// swizzle left value
	v1["x"_sw] = 3.0;
	std::cout << "v1:" << v1 << std::endl;

	sf::vec4 v3(2, 1, 4, 1);
	std::cout << "v2:" << v2 << std::endl;
	std::cout << "v3:" << v3 << std::endl;


	//// swizzle right value, swizzle left value
	v3["yw"_sw] = v2["xz"_sw];
	std::cout << "Result:" << v3 << std::endl;


	sf::vec4 v4(2, sf::vec2(1, 3), 3);
	std::cout << "Result:" << v4 << std::endl;

	float w = v4["w"_sw];

	sf::vec3 a{ 1,2,3 };
	sf::vec3 b{ 4,5,6 };

	float d = sf::dot(a, b);

	_unroll_check();


	SquareKernel kern;
	kern.setup_cpu();
	kern.count = N;

	sf::CPUBackend backend;
	backend.execute(kern, N);
	std::cout << kern.outData[10] << '\n';   // prints 10
}
