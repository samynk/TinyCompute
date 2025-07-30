cmake_minimum_required(VERSION 3.24)
project(SwizzleForge
        VERSION 0.1.0
        DESCRIPTION "Regex-based single-source C++ → Compute-Shader framework"
        LANGUAGES CXX)

include(FetchContent)
include(${CMAKE_SOURCE_DIR}/util.cmake)

# ─────────────────────────────────────────────────────────────
# Build options
# ─────────────────────────────────────────────────────────────
option(SWIZZLEFORGE_BUILD_SHARED  "Build SwizzleForge as a shared library" ON)
option(SWIZZLEFORGE_BUILD_TESTS   "Build GoogleTest unit tests"            ON)
option(SWIZZLEFORGE_BUILD_EXAMPLES "Build example applications"           ON)

# Global C++ standard & warnings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch GLM
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.1
   GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)
set_target_properties(glm PROPERTIES FOLDER "3rd‑party/GLM")

# Fetch GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.4  # Use the desired GLFW version
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)
set_target_properties(glfw PROPERTIES FOLDER "3rd‑party/GLFW3")
set_target_properties(uninstall PROPERTIES FOLDER "3rd‑party/GLFW3")
set_target_properties(update_mappings PROPERTIES FOLDER "3rd‑party/GLFW3")

# Fetch GLEW
FetchContent_Declare(
  glew
  GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
  GIT_TAG        glew-cmake-2.2.0  # Use the desired GLEW version
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glew)
set_target_properties(libglew_shared PROPERTIES FOLDER "3rd‑party/GLEW")
set_target_properties(libglew_static PROPERTIES FOLDER "3rd‑party/GLEW")

# Fetch stb
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        master  # Or specify a specific commit or tag
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(stb)

# ─────────────────────────────────────────────────────────────
# Sub-directories
# ─────────────────────────────────────────────────────────────
add_subdirectory(swizzleforge)
add_subdirectory(transpiler)
add_subdirectory(opengl)

if (SWIZZLEFORGE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if (SWIZZLEFORGE_BUILD_EXAMPLES)
    add_subdirectory(examples)
    add_subdirectory(Project00)
    set_target_properties(Project00 PROPERTIES FOLDER "03‑Examples")
    add_subdirectory(Projects/Project01)
    set_target_properties(Project01 PROPERTIES FOLDER "03‑Examples")
endif()


# After you call add_subdirectory(llvm)  — or whatever brings them in
foreach(t
    acc_gen
    omp_gen
    vt_gen
    intrinsics_gen
    clang-tablegen-targets
    ARMTargetParserTableGen
    AArch64TargetParserTableGen
    RISCVTargetParserTableGen)
  if (TARGET ${t})
    set_target_properties(${t} PROPERTIES FOLDER "3rd‑party/LLVM")
  endif()
endforeach()

foreach(g gtest gtest_main gmock gmock_main)
  if (TARGET ${g})
    set_target_properties(${g} PROPERTIES FOLDER "3rd‑party/gtest")
    # Or to hide entirely:
    # set_property(TARGET ${g} PROPERTY EXCLUDE_FROM_ALL 1)
  endif()
endforeach()

set_target_properties(swizzleforge           PROPERTIES FOLDER "01-Core")
set_target_properties(sf_transpile           PROPERTIES FOLDER "02‑Tools")
set_target_properties(swizzleforge_demo      PROPERTIES FOLDER "03‑Examples")
set_target_properties(swizzleforge_tests     PROPERTIES FOLDER "04‑Tests")