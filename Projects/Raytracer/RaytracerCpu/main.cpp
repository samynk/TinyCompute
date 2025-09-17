#include <iostream>
#include <vector>
#include "vec.hpp"
#include "ComputeWindow.hpp"
#include "RayTracerWindow.hpp"
#include <string>
#include <iostream>

int main() {
	try {
		ComputeWindow<RayTracerWindow> window{ 640,480,"CPU Raytracer" };
		window.init();
		window.renderLoop();
	}
	catch (const std::exception& e) {
		std::cerr << "An exception occurred: " << e.what() << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}
	return 0;
}



