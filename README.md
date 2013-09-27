# A simple ray tracer

This is our projects repository for computer graphics course.

## Dependencies
  
The following packages are required:
* gcc 4.7 or clang 3.1
* CMake 2.6
* SDL
* Boost
* libgd

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
