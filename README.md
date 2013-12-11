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

## Usage instructions

The help is provided with "--help" command line argument. Use "--bpt" to enable
bidirectional path tracing and use "-s" to set the number of samples. The
higher the number of samples the longer the rendering will take. The default
value is 5.

## New features

* Support for area and directional lights.
* Bidirectional path tracing algorithm.
* Blender exporter to slightly modified NFF. Does not fully support materials,
  textures and lights, but provides a nice way to export geometry to NFF.

The following imagee was rendered with options "--bpt -s 1000" and took around
6 hours to render on 4 core i7. ![Cornell box](https://raw.github.com/Jaak/ray/master/imgs/cornell.png)
