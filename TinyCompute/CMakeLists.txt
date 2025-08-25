set(TINYCOMPUTE_SRC
    "vec.hpp"
    "swizzle.hpp"
    "kernel_intrinsics.hpp"
    "computebackend.hpp"
    "math/arithmetic.hpp"  "images/ImageFormat.hpp" "math/linearalgebra.hpp")

add_library(
    TinyCompute
    INTERFACE
        ${TINYCOMPUTE_SRC}
)

target_include_directories(
    TinyCompute 
    INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)
