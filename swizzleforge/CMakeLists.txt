# Collect all library sources (adjust the list to your actual files)
set(SWIZZLEFORGE_SRC
    "vec.hpp"
    "annotations.hpp"
    "swizzle.hpp"
    "shader_traits.hpp"
    "regex_transpiler.cpp"
    "kernel_intrinsics.hpp"
)

find_package(OpenGL REQUIRED)

add_library(swizzleforge
    ${SWIZZLEFORGE_SRC}
)

# Toggle STATIC vs SHARED according to top-level option
if (SWIZZLEFORGE_BUILD_SHARED)
    set_target_properties(swizzleforge PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        POSITION_INDEPENDENT_CODE  ON
        RUNTIME_OUTPUT_DIRECTORY   "${CMAKE_BINARY_DIR}/bin"
    )
else()
    set_target_properties(swizzleforge PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
endif()

target_include_directories(swizzleforge
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        ${OPENGL_INCLUDE_DIRS} 
        ${glew_SOURCE_DIR}/include 
)

target_compile_features(swizzleforge PUBLIC cxx_std_20)
