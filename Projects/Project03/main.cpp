#include <iostream>
#include <vector>
#include "vec.hpp"
#include "ComputeWindow.hpp"
#include "GameOfLifeWindow.h"
#include <string>
#include <iostream>



int main() {
	try {
		ComputeWindow<GameOfLifeWindow> window{ 1024,1024,"Compute Shader Tutorial" };
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



