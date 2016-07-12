# A simple ray tracer

This is personal project for toying around with software raytracers. Used to
support files in the NFF format but now, because it supports path tracing, uses
some custom NFF amalgamation. Currently the best renderer is VCM. BPT is also
supported but the code is clearly broken.

## Installation instruction

### Dependencies
  
The following packages are required:
* gcc 4.7 or clang 3.1
* CMake 2.6
* Boost (thread, date\_time, program\_options)
* libgd (optional for generating PNG images)

On debian based system the packages could be installed as follows:
<pre>
$ sudo apt-get install gcc cmake libboost-dev libgd-dev
</pre>

On gentoo make sure that png USE flag is enabled and as root:
<pre>
$ emerge -a1 cmake gd boost
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
bidirectional path tracing (quite broken) or "--vcm" for vertex connection and
merging rendering algorithm. Use "-s" to set the number of samples. The higher
the number of samples the longer the rendering will take. The default value is
5.

### Example renders

![Cornell box](https://raw.github.com/Jaak/ray/master/imgs/cornell.png)

![Cornell box with spheres](https://raw.github.com/Jaak/ray/master/imgs/directional.png)
