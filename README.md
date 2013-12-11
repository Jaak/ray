# A simple ray tracer

This is our projects repository for computer graphics course.

## Installation instruction

### Dependencies
  
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

On gentoo make sure that png USE flag is enabled and as root:
<pre>
$ emerge -a1 cmake libsdl gd boost
</pre>

### Build instructions

In the root source directory:
<pre>
$ cp config.local.example config.local
$ mkdir build
$ cd build
$ cmake ..
$ make -j3
$ ./src/ray --help
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

### Concerning importers/exporters

* An implementation for Blender exporter for NFF format.

This task is a good opportunity to familiarize oneself with Blender Python API modules, especially bpy.data, and writing scripts for Blender. The first step would be to use this API for extracting the data followed by writing it into a .nff file (using Python I/O). The task should not be very difficult since there are several codes for Blender exporters avalable which can be used for guidance, e.g see [DirectX exporter](https://projects.blender.org/tracker/index.php?func=detail&aid=22795). 

* .obj exporter for the ray tracer.

Since we want the ray tracer to support at least one of the most widely used file formats, we are planning to implement an .obj importer. The first step would be to implement a simple test programme for reading in data from the .obj file and putting it into the data structures similar to the ones the ray tracer uses. Then the programme can be integrated to the ray tracer. Another thought is to additionally implement a 3ds format importer. Howeve, this might be a bit more time-consuming since 3ds is in a binary format. Both OBJ ja 3ds formats are well documented and have C++
libraries.
