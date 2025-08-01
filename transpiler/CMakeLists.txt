add_executable(sf_transpile
    transpile_main.cpp
  "KernelValidator.h" "TranspileAction.h" "matchers/LocalSizeCallback.h"  "matchers/KernelLocatorCallback.h" "callbacks/VarCallback.h")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
target_compile_features(sf_transpile PUBLIC cxx_std_20)

target_compile_definitions(sf_transpile PRIVATE NDEBUG _ITERATOR_DEBUG_LEVEL=0)

if (MSVC)
  target_compile_options(sf_transpile PRIVATE
    "/MD"  # use release DLL CRT
  )
endif()

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(SYSTEM
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

add_definitions(${LLVM_DEFINITIONS})
set(LLVM_LINK_COMPONENTS support)

# Optional: silence some warnings
add_compile_options(-fno-rtti)

target_link_libraries(
    sf_transpile
    PRIVATE
    clangAST
    clangFrontend
    clangTooling
)
# Optional install step if you later want `make install`
install(TARGETS sf_transpile RUNTIME DESTINATION tools)
