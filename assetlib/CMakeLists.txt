set(AssetLibProject "AssetLib")
add_library(${AssetLibProject} STATIC
	"ImageLoader.h"
	"ImageLoader.cpp"
)

target_link_libraries(${AssetLibProject} PUBLIC TinyCompute)
target_include_directories(${AssetLibProject} PUBLIC 
	${stb_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(${AssetLibProject} PROPERTIES FOLDER "01-Core")
