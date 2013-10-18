# A simple ray tracer

This is our projects repository for computer graphics course.

## Dependencies
  
The following packages are required:
* gcc 4.7 or clang 3.1
* CMake 2.6
* SDL
* Boost (thread, date\_time, program\_options)
* libgd (optional for generating PNG images)

On debian based system the packages could be installed as follows:
<pre>
$ sudo apt-get install gcc cmake libsdl1.2-dev libboost-dev libgd-dev
</pre>

## Build instructions

In the root source directory:
<pre>
$ mkdir build
$ cd build
$ cmake ..
$ make -j3 # Or higher values if you have more cores than two
$ ./src/ray ../nff/tetra7.nff
</pre>

## Goals for Computer Graphics course

In the absolute minimal set of features we will implement:
* Add support for area and/or directional lights.
* Implement path tracing algorithm for global illumination.
* Improve support for models. Right now the basic NFF format is supported, this
  is quite limiting as it's difficult to write and finds models for testing
  purposes.

The last goal is intentionally quite vague as at the moment we lack the
research on better input formats. The first goal is to actually do the minimal
amount research on implementing blender exporter, the 3ds and the .obj formats.
At the moment I lean towards extending the NFF format and implement blender
exporter for that format. Potentially NFF format might be too limiting or
inefficient so a custom input format might turn out to be required. Blender has
extremely complicated and poorly documented format, and writing importer for
that is not feasible. Alternative good option is to implement .obj or 3ds
format importer. Both formats are well documented and already have C++
libraries for.

The goal for path tracer is also minimal. Ideally we implement bidirectional
path tracing and use some standard tricks to make it faster. The low-hanging
fruit is to use importance sampling. If we go crazy metropolis light transport
can be implemented. A good overview of path tracing and some acceleration
methods are discussed in a
[video](https://www.youtube.com/watch?v=QhJhVkbCgVU "Bidirectional Path Tracing").
