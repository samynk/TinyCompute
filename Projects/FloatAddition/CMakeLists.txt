set(ProjectName "AddFloats")
add_executable(${ProjectName}
    main.cpp
    "AddFloats.h" 
    "GameOfLifeWindow.h" 
    "GameOfLifeWindow.cpp")

configureTinyCompute(${ProjectName} "03-Examples/A-AddFloats" "AddFloats.h")