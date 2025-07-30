add_executable(Project01
    main.cpp
 "GameOfLife.h")

target_compile_features(Project01 PUBLIC cxx_std_20)

# Source kernel and generated outputs
set(KERNEL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/GameOfLife.h)
set(GLSL_OUT    ${CMAKE_CURRENT_BINARY_DIR}/GameOfLife.glsl)
set(SPIRV_OUT   ${CMAKE_CURRENT_BINARY_DIR}/GameOfLive.spv)

# 1) Transpile C++  →  GLSL
add_custom_command(
    TARGET Project01
    COMMAND sf_transpile  ${KERNEL_SRC} -o ${GLSL_OUT}
    DEPENDS sf_transpile  ${KERNEL_SRC}
    COMMENT "Transpiling ${KERNEL_SRC}"
    USES_TERMINAL 
)

target_link_libraries(Project01
    PRIVATE
        swizzleforge
        ComputeLib
)

# Put demo executable next to shaders / assets
set_target_properties(Project01 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_dependencies(Project01 sf_transpile ComputeLib) 
