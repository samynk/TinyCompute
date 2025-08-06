find_package(OpenGL REQUIRED)

add_library(ComputeLibOpenGL STATIC
    "SurfaceRenderer.h"  
	"ComputeShader.h" 
	"GLImage.h" 
	"ComputeWindow.h"  
	"ShaderBuffer.h"   
	"Compute.h" 
	"ImageBuffer2D.h" 
	"ImageFormat.h"
	"stb_image.h"
	"SurfaceRenderer.cpp"  
	"ComputeShader.cpp"
	"GLImage.cpp"
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
		
		glfw 
		libglew_static 
		glm
	INTERFACE
		swizzleforge
) 

 set_target_properties(ComputeLibOpenGL PROPERTIES
        FOLDER "01-Core" )