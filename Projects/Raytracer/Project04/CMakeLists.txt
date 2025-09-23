set(ProjectName "CameraRays")
add_executable(${ProjectName}
    main.cpp
    "RayTracer01.h" 
    "RayTracerWindow.h" 
    "RayTracerWindow.cpp"
)

 configureTinyCompute(${ProjectName} "03-Examples/B-Raytracer" "RayTracer01.h" )
