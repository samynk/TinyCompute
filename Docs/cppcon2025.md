---
marp: false
theme: uncover
_class: invert
paginate: true
---

![bg](images/From%20Pure%20ISO%20C++20%20To%20Compute%20Shaders_Title_Card%20copy.png)

---
# Introduction

* Koen Samyn
   * Program coordinator Game Development at Digital Arts and Entertainment (Belgium)
   * Programming generalist 
   * Educator : C++ / Python / Math / Graphics programming
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
---
# Is this new?
* A lof of movement in this area and existing frameworks
    * CUDA /HIP
    * OpenCL
    * CompuShady (Python)
* Niche I am trying to fill:
    * Non-proprietary
    * Lightweight
    * Educational focus: teach transferable skills.
---
# Design goals
* Start with C++20 --> no new keywords 
* Transpile C++ code into GLSL
* Execution backend for cpu and gpu (OpenGL compute shaders)
* Make it easier to manage typical gpu resources: buffers, images, uniform variables
* Debug on CPU -> Test on GPU with a change of backend.
* Support for swizzle syntax 
* Compact code 
