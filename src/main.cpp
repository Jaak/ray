#include "scene.h"
#include "png_surface.h"
#include "sdl_surface.h"
#include "kdtree_primitive_manager.h"
#include "naive_primitive_manager.h"
#include "nff_scene_reader.h"

#include <iostream>
#include <string>
#include <cstdlib>

/**
 * TODO:
 * Major:
 *  - path tracing (bidirectional)
 *  - improved input format
 *
 * Minor:
 *  - better command line options...
 *  - we can compute if the intersection was internal or external from
 *    ray direction and the normal...
 *  - move operator<< implementations to cpp files. this way it's sufficient to
 *    include <iosfwd>
 *  - The code is still not quite C++11 idiomatic.
 */

int main(int argc, char** argv) {
  if (argc < 2 || argc > 3) {
    std::cout << "Usage:\n"
              << "\t" << argv[0] << " "
              << "[nff file] [destination file]\n"
              << "If no destination file is supplied then "
              << "\"out.png\" will be used instead." << std::endl;
    return EXIT_SUCCESS;
  }

  Scene scene;
  std::string file;

  if (argc != 3) {
    file = "out.png";
  } else {
    file = argv[2];
  }

  //scene.attachSurface(new PngSurface(file));
  scene.attachSurface(new SDLSurface());
  scene.setPrimitiveManager(new KdTreePrimitiveManager());
  scene.setSceneReader(new NFFSceneReader(argv[1]));
  scene.init();
  scene.run();
  return EXIT_SUCCESS;
}
