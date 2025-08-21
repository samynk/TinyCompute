add_library(AssetLib STATIC
	"ImageLoader.h"
	"ImageLoader.cpp"
)

target_link_libraries(AssetLib PUBLIC TinyCompute)
target_include_directories(AssetLib PUBLIC 
	${stb_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)
