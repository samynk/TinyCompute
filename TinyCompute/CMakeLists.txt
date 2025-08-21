set(SWIZZLEFORGE_SRC
    "vec.hpp"
    "swizzle.hpp"
    "kernel_intrinsics.hpp"
    "computebackend.hpp"
    "math/arithmetic.hpp"  "images/ImageFormat.h")

add_library(
    swizzleforge 
    INTERFACE
        ${SWIZZLEFORGE_SRC}
)

target_include_directories(
    swizzleforge 
    INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)
