---
marp: false
theme: uncover
_class: invert
paginate: true
---

![bg](images/From%20Pure%20ISO%20C++20%20To%20Compute%20Shaders_Title_Card%20copy.png)

---
# Introduction
``` glsl
#version 430

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding=0,r8ui) uniform uimage2D inData; 
layout(binding=1,rgba8) uniform image2D outData; 

void main()
{
    uvec2 gId = gl_GlobalInvocationID.xy;
    ivec2 coordinate = ivec2(gId.x, gId.y);

    if (imageLoad(inData, coordinate).x > 0)
    {
        imageStore(outData, coordinate, vec4(34.0f / 255.0f, 177.0f / 255.0f, 76 / 255.0f, 1.0f));
    }
    else
    {
        imageStore(outData, coordinate, vec4(0, 0, 0, 1.0f));
    }
}
```
---
# Why this project?
* Mental load to develop compute shaders is heavy
    * Still developing C++ skills.
    * Management layer needed for gpu resources.
    * Debugging gpu programs is hard.
* Programming model is different 
    * Workgroups / threads
    * Homogeneous parallellism: every thread performs the same task
    * Existing C++ code can be hard to convert
