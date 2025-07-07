#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{

	std::cout << "Tool called with " << (argc-1) << " files." << std::endl;

    std::vector<std::string> fileList;
    std::cout << "Arguments:" << std::endl;
    for (int i = 1; i < argc; ++i) {
        std::cout << "Argument " << i << ": " << argv[i] << std::endl;
        fileList.emplace_back(argv[i]);
    }

    return 0;
}