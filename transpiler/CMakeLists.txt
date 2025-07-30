add_executable(sf_transpile
    transpile_main.cpp
 )

target_compile_features(sf_transpile PUBLIC cxx_std_20)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(SYSTEM
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

add_definitions(${LLVM_DEFINITIONS})

# Optional: silence some warnings
add_compile_options(-fno-rtti)

target_link_libraries(
    sf_transpile
    PRIVATE
    clangTooling
    clangBasic
    clangASTMatchers
    ${llvm_libs}
)

# Optional install step if you later want `make install`
install(TARGETS sf_transpile RUNTIME DESTINATION tools)
