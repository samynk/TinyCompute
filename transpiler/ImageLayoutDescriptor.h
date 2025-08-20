#pragma once
#include <string>

struct ImageFormatDescriptor{
	// rgba8, r32, ...
	std::string imageType;
	// for unsigned int images, no prefix
	// for float : 'f'
	// for signed int: 'i'
	std::string imagePrefix;
};