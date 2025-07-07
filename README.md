Swizzleforge is a C++ library which allows developers to write compute shaders in pure C++. 

The major parts of the library are:

* a simple OpenGL compute shader abstraction
* support for swizzling with the aid of the array index operator and a custom string value (with suffix _sw)
* support for compute kernels that can be executed with C++ algorithms in a parallel or serial fashion.
* support for defining buffers and their bindings.

Finally C++ compute kernels can be easily converted into glsl compute shaders with the help of simple regular expressions.
