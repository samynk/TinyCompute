set(ProjectName "RaytracerCPU")
add_executable(${ProjectName}
    main.cpp
 "RayTracerCPU.hpp" "RayTracerWindow.hpp" "RayTracerWindow.cpp")

configureTinyCompute(${ProjectName} "03-Examples/B-Raytracer" "RayTracerCPU.hpp")