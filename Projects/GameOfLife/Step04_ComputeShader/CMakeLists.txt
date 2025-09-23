set(ProjectName "GOL_04")
add_executable(${ProjectName}
    main.cpp
 "GameOfLife.h" "GameOfLifeWindow.h" "GameOfLifeWindow.cpp")

configureTinyCompute(${ProjectName} "03-Examples/C-GameOfLife" "GameOfLife.h")
