set(ProjectName "Project03")
add_executable(${ProjectName}
    main.cpp
 "GameOfLife.h" "GameOfLifeWindow.h" "GameOfLifeWindow.cpp")

 configureTinyCompute(${ProjectName} "03-Examples/D-GameOfLife" "GameOfLife.h")
