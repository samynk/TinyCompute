#include <iostream>
#include <vector>
#include <string>

#include "vec.hpp"
#include "ComputeWindow.hpp"
#include "GameOfLifeWindow.h"




int main() {
	try {
		ComputeWindow<GameOfLifeWindow> window{ 512,512,"Compute Shader Tutorial" };
		window.init();
		window.renderLoop();
		window.close();
	}
	catch (const std::exception& e) {
		std::cerr << "An exception occurred: " << e.what() << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}
	return 0;
}



