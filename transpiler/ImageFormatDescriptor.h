#pragma once
#include <string>

#include "images/ImageFormat.h"

struct ImageFormatDescriptor{
	// rgba8, r32, ...
	std::string imageIdentifier;
	// for unsigned int images, no prefix
	// for float : 'f'
	// for signed int: 'i'
	tc::Scalar scalar;
};