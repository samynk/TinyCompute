find_package(OpenGL REQUIRED)

add_library(ComputeLibOpenGL STATIC
    "SurfaceRenderer.h"  
	"ComputeShader.h" 
	 
	"ComputeWindow.h"  
	   
	"Compute.h" 
	 
	
	
	"SurfaceRenderer.cpp"  
	"ComputeShader.cpp"
	
 "OpenGLBackend.h")

target_include_directories(ComputeLibOpenGL PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENGL_INCLUDE_DIRS} 
    ${stb_SOURCE_DIR}
	${glew_SOURCE_DIR}/include 
)

target_link_libraries(
	ComputeLibOpenGL 
	PRIVATE 
		OpenGL::GL 
		
	PUBLIC
		TinyCompute
		glfw 
		libglew_static 
		glm
) 

 set_target_properties(ComputeLibOpenGL PROPERTIES
        FOLDER "01-Core" )