#pragma once
#include <clang/Basic/SourceLocation.h>

struct PendingEdit { 
	clang::SourceRange range; 
	std::string replacement; 
};


