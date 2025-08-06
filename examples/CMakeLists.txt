add_executable(swizzleforge_demo
    main.cpp
)

target_compile_features(swizzleforge_demo PUBLIC cxx_std_20)
target_link_libraries(
    swizzleforge_demo
    swizzleforge
)

set_target_properties(swizzleforge_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_dependencies(swizzleforge_demo sf_transpile) 
