add_library(AssetLib STATIC
	"ImageLoader.h"
	"ImageLoader.cpp"
)

add_library(stb INTERFACE)
target_link_libraries(AssetLib PUBLIC swizzleforge)
target_include_directories(AssetLib PUBLIC 
	${stb_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)
