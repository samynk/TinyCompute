include(FetchContent)

# Fetch GoogleTest at configure time
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
# Avoid overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(TestProject "TinyComputeTest")
add_executable(
    ${TestProject} 
    "vec_tests.cpp"
    "transpiler_tests.cpp"
    "pixel_tests.cpp")

target_compile_features(${TestProject} PUBLIC cxx_std_20)

target_link_libraries(${TestProject} 
        gtest_main
       TinyCompute
)

set_target_properties(${TestProject} PROPERTIES FOLDER "04‑Tests")

add_test(NAME SwizzleForgeUnitTests COMMAND swizzleforge_tests)
