set(ProjectName "GOL_03")
add_executable(${ProjectName}
    main.cpp
    Kernel.hpp Kernel.cpp
)

target_compile_features(${ProjectName} PUBLIC cxx_std_20)
target_include_directories(${ProjectName} PRIVATE ${pge_SOURCE_DIR})

set_target_properties(${ProjectName} PROPERTIES FOLDER "03-Examples/A-GameOfLife")