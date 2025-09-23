set(ProjectName "GOL_05")
add_executable(${ProjectName}
    main.cpp
 "GameOfLife.h" "GameOfLifeWindow.h" "GameOfLifeWindow.cpp")

configureTinyCompute(${ProjectName} "03-Examples/C-GameOfLife" "GameOfLife.h")
