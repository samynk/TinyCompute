add_executable(swizzleforge_demo
    main.cpp
)

# Source kernel and generated outputs
set(KERNEL_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
set(GLSL_OUT    ${CMAKE_CURRENT_BINARY_DIR}/main.glsl)
set(SPIRV_OUT   ${CMAKE_CURRENT_BINARY_DIR}/main.spv)

# 1) Transpile C++  →  GLSL
add_custom_command(
    TARGET swizzleforge_demo
    COMMAND sf_transpile  ${KERNEL_SRC} -o ${GLSL_OUT}
    DEPENDS sf_transpile  ${KERNEL_SRC}            # <- important!
    COMMENT "Transpiling ${KERNEL_SRC}"
    USES_TERMINAL 
)

target_link_libraries(swizzleforge_demo
    PRIVATE
        swizzleforge
        ComputeLib
)

# Put demo executable next to shaders / assets if you like:
set_target_properties(swizzleforge_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_dependencies(swizzleforge_demo sf_transpile ComputeLib) 
