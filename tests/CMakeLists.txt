include(FetchContent)

# Fetch GoogleTest at configure time
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
# Avoid overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(swizzleforge_tests
    vec_tests.cpp
    transpiler_tests.cpp
)

target_compile_features(swizzleforge_tests PUBLIC cxx_std_20)

target_link_libraries(swizzleforge_tests
    PRIVATE
        gtest_main
        swizzleforge
)

add_test(NAME SwizzleForgeUnitTests COMMAND swizzleforge_tests)
