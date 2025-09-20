set(TCTranspileProject "TinyComputeTranspile")
add_executable(${TCTranspileProject}
    transpile_main.cpp
  "KernelValidator.h" "TranspileAction.h" "matchers/LocalSizeCallback.h"  "matchers/KernelLocatorCallback.h"  "callbacks/BindingPointCallback.h" "PendingEdit.h"  "callbacks/CpuMethodCallback.h" "KernelStruct.h" "KernelRewriter.h" "KernelRewriter.cpp" "ImageFormatDescriptor.h")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_compile_features(${TCTranspileProject} PUBLIC cxx_std_20)
target_compile_definitions(${TCTranspileProject} PRIVATE NDEBUG _ITERATOR_DEBUG_LEVEL=0)

if (MSVC)
  target_compile_options(${TCTranspileProject} PRIVATE
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
    ${TCTranspileProject}
    PRIVATE
    TinyCompute
    clangAST
    clangFrontend
    clangTooling
)
install(TARGETS ${TCTranspileProject} RUNTIME DESTINATION tools)
set_target_properties(${TCTranspileProject} PROPERTIES FOLDER "02‑Tools")