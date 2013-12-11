#include "kdtree_primitive_manager.h"
#include "naive_primitive_manager.h"
#include "nff_scene_reader.h"
#include "pathtracer.h"
#include "raytracer.h"
#include "scene.h"
#include "sdl_surface.h"
#include "tga_surface.h"
#include "uniform_sampler.h"

#ifdef HAVE_GD_SUPPORT_PNG
  #include "png_surface.h"
#endif

#include <boost/program_options.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace po = boost::program_options;

void parseCommandLine(int argc, char** argv, po::options_description& desc, po::variables_map& vm) {
  desc.add_options()
    ("help,h",                              "Output this help message")
#ifdef HAVE_GD_SUPPORT_PNG
    ("png",       po::value<std::string>(), "Output PNG image")
#endif
    ("tga",       po::value<std::string>(), "Output TGA image")
    ("bpt",                                 "Use bidirection path tracer")
    ("samples,s", po::value<size_t>(),      "Number of samples per pixel")
    ("input,i",   po::value<std::string>(), "Input NFF file")
    ("non-interactive",                     "Don't render interactively");

  po::positional_options_description p;
  p.add("input", -1);

  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  po::notify(vm);
}

/**
 * TODO:
 * Major:
 *  - Improved input format (.obj, .3ds, or custom blender exporter?)
 *
 * Minor:
 *  - All of the init-s are a code smell... Initialization should always be done in the class constructor.
 *  - The first step in implementing better importer is to rename SceneReader to SceneImporter
 *    and make it an actual virtual class. The importer should just implement various virtual
 *    methods that builds the scene.
 *  - We can compute if the intersection was internal or external from ray direction and the normal...
 *  - Move operator<< implementations to cpp files. this way it's sufficient to include <iosfwd>
 *  - The code is still not quite C++11 idiomatic.
 */

int main(int argc, char** argv) {
  Scene scene;
  const std::string name = "Command line options:";
  po::options_description desc (name);
  po::variables_map vm;
  parseCommandLine(argc, argv, desc, vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("input")) {
    std::string inp_file = vm["input"].as<std::string>();
    scene.setSceneReader(new NFFSceneReader(inp_file.c_str()));
  }
  else {
    std::cerr << "No input file specified!" << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count("non-interactive") == 0) {
    scene.attachSurface(new SDLSurface());
  }

  if (vm.count("bpt") == 0) {
    scene.setRenderer(new Raytracer (scene));
  }
  else {
    scene.setRenderer(new Pathtracer (scene));
  }

  if (vm.count("samples") == 0) {
    scene.setSampler(new UniformSampler(5));
  }
  else {
    scene.setSampler(new UniformSampler(vm["samples"].as<size_t>()));
  }

#ifdef HAVE_GD_SUPPORT_PNG
  if (vm.count("png")) {
    std::string out_file = vm["png"].as<std::string>();
    scene.attachSurface(new PngSurface(out_file));
  }
#endif

  if (vm.count("tga")) {
    std::string out_file = vm["tga"].as<std::string>();
    scene.attachSurface(new TgaSurface(out_file));
  }

  // TODO: allow selection of various primitive managers
  scene.setPrimitiveManager(new KdTreePrimitiveManager());
  scene.init();
  scene.run();
  return EXIT_SUCCESS;
}
