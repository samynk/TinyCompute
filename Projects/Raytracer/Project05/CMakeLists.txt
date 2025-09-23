set(ProjectName "RaytracerGPU")
add_executable(${ProjectName}
    main.cpp
 "RayTracer02.h" "RayTracerWindow.h" "RayTracerWindow.cpp")

configureTinyCompute(${ProjectName} "03-Examples/B-Raytracer" "RayTracer02.h")