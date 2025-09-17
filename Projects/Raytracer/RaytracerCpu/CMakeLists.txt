set(ProjectName "RaytracerCPU")
add_executable(${ProjectName}
    main.cpp
 "RayTracerCPU.hpp" "RayTracerWindow.hpp" "RayTracerWindow.cpp")

 SET_WORKING_DIR_TO_RESOURCES(${ProjectName})
target_compile_features(${ProjectName} PUBLIC cxx_std_20)

# Source kernel and generated outputs
set(KERNEL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/RayTracerCPU.hpp)
set(GLSL_OUT    ${CMAKE_CURRENT_BINARY_DIR})
set(SPIRV_OUT   ${CMAKE_CURRENT_BINARY_DIR})

set(TRANSPILER_FLAGS_FILE "${CMAKE_CURRENT_BINARY_DIR}/sf_transpile_args.txt")
file(WRITE ${TRANSPILER_FLAGS_FILE}
  "-std=c++20\n"
  "-I${CMAKE_CURRENT_SOURCE_DIR}/include\n"
  "-x\n"
  "-c++\n"
  "-fms-compatibility\n"
  "-fms-extensions\n"
)

get_target_property(TC_INCLUDE_DIR TinyCompute SOURCE_DIR)
# 1) Transpile C++  →  GLSL
add_custom_command(
    TARGET ${ProjectName}
    COMMAND sf_transpile  ${KERNEL_SRC} -o ${GLSL_OUT} -- -I${TC_INCLUDE_DIR} -std=c++20 -x c++ -fms-compatibility -fms-extensions
    DEPENDS sf_transpile  ${KERNEL_SRC} ${TRANSPILER_FLAGS_FILE}
    COMMENT "Transpiling ${KERNEL_SRC}"
    USES_TERMINAL 
)

add_custom_command(TARGET ${ProjectName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Resources"
            "$<TARGET_FILE_DIR:${ProjectName}>/")

target_link_libraries(${ProjectName}
PUBLIC
    ComputeLibOpenGL
	AssetLib
    TinyCompute
)

# Put demo executable next to shaders / assets
#set_target_properties(Project01 PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
#)

add_dependencies(${ProjectName} sf_transpile ComputeLibOpenGL) 
set_target_properties(${ProjectName} PROPERTIES FOLDER "03-Examples/B-Raytracer")
