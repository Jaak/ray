#include "camera.h"
#include "common.h"
#include "light.h"
#include "parser.h"
#include "pixel.h"
#include "primitive_manager.h"
#include "renderer.h"
#include "sampler.h"
#include "scene.h"
#include "scene_reader.h"

#include <boost/date_time.hpp>
#include <boost/thread.hpp>
#include <boost/mpl/if.hpp>
#include <string>

Scene::Scene() {
    m_background = m_materials.registerMaterial (Material { Colour { 0, 0, 0 } });
}

void Scene::setPrimitiveManager(PrimitiveManager* pm) {
  m_manager = std::unique_ptr<PrimitiveManager>(pm);
}

void Scene::setBackground(const Colour& c) {
    m_background = m_materials.registerMaterial (Material { c });
}

const Material& Scene::background() const {
    return m_materials[m_background];
}

void Scene::setSceneReader(SceneReader* sr) {
  m_scene_reader = std::unique_ptr<SceneReader>(sr);
}

void Scene::setRenderer (Renderer* r) {
  m_renderer = std::unique_ptr<Renderer>(r);
}

void Scene::setSampler (Sampler* s) {
  m_sampler  = std::unique_ptr<Sampler>(s);
}

std::string Scene::getFname() {return (*m_scene_reader).getFname();}

Scene::~Scene() { }

class Block {
public: /* Methods: */

  Block(int y0, int y1, int x0, int x1)
    : m_y0(y0), m_y1(y1), m_x0(x0), m_x1(x1)
  { }

  void renderRough(Scene& scene) const {
    const auto c = scene.sampler().samplePixel (m_x0, m_y0);
    for (auto& surface : scene.surfaces()) {
      for (int h = m_y0; h < m_y1; ++h) {
        for (int w = m_x0; w < m_x1; ++w) {
          surface->setPixel(h, w, c);
        }
      }
    }
  }

  void renderNice(Scene &scene) const {
    for (auto &surface : scene.surfaces()) {
      for (int h = m_y0; h < m_y1; ++h) {
        for (int w = m_x0; w < m_x1; ++w) {
          const auto c = scene.sampler().samplePixel (w, h);
          surface->setPixel(h, w, c);
        }
      }
    }
  }

  size_t area () const {
      return (m_y1 - m_y0) * (m_x1 - m_x0);
  }

  void subdiv (size_t numPixels, std::vector<Block>& blocks, size_t rot = 0) {
      if (area () <= numPixels) {
          blocks.emplace_back (m_y0, m_y1, m_x0, m_x1);
          return;
      }

      const auto ym = m_y0 + (m_y1 - m_y0) / 2;
      const auto xm = m_x0 + (m_x1 - m_x0) / 2;
      Block bs [4] {
          { m_y0, ym, m_x0, xm } // top-left
        , { ym, m_y1, m_x0, xm } // bottom-left
        , { ym, m_y1, xm, m_x1 } // bottom-right
        , { m_y0, ym, xm, m_x1 } // top-right
      };

      for (size_t i = 0; i < 4; ++ i) {
          const auto j = (rot + i) % 4;
          const auto newRot = (rot + 2) % 4;
          bs[j].subdiv (numPixels, blocks, newRot);
      }
  }

private: /* Fields: */
  const int m_y0, m_y1, m_x0, m_x1;
};

enum TaskType { Rough = 0, Nice };

class Task {
public: /* Methods: */
  
  Task(size_t i, size_t nP, std::vector<Block>& blocks, Scene& scene, TaskType type)
    : m_i(i)
    , m_nP(nP)
    , m_blocks(blocks)
    , m_scene(scene)
    , m_type(type)
  { }

  void operator()() {
    int i = m_i;
    int end = m_blocks.size();
    switch (m_type) {
      case Rough:
        for (; i < end; i += m_nP)
          m_blocks[i].renderRough(m_scene);
        break;
      case Nice:
        for (; i < end; i += m_nP)
          m_blocks[i].renderNice(m_scene);
        break;
    }
  }

private: /* Fields: */
  const size_t         m_i;
  const size_t         m_nP;
  std::vector<Block>&  m_blocks;
  Scene&               m_scene;
  const TaskType       m_type;
};

void Scene::init() {
  m_scene_reader->init(*this);
  for (auto& surface : m_surfaces) {
    surface->init();
  }

  floating acc = 0.0;
  for (auto& light : m_lights) {
    acc += luminance (light->intensity ());
  }

  if (acc > 0.0) {
    acc = 1.0 / acc;
    for (auto& light : m_lights) {
      light->setSamplingPr(luminance (light->intensity())*acc);
    }
  }

  m_camera.init();
  m_manager->init();
  m_sampler->setCamera (m_camera);
  m_sampler->setRenderer (*m_renderer);
}

void Scene::addPrimitive(const Primitive* p) { m_manager->addPrimitive(p); }

void Scene::addLight(Light* l) { m_lights.emplace_back (l); }

const std::vector<std::unique_ptr<Light>>& Scene::lights() const { return m_lights; }

void Scene::run() {
  using namespace boost::posix_time;

  const auto start_time = microsec_clock::local_time();

  const size_t nP = std::max(boost::thread::hardware_concurrency(), 1u);

  std::cout << "Running on " << nP << " threads..." << std::endl;

  Block entireScreen{ 0, m_camera.height(), 0, m_camera.width() };
  std::vector<Block> blocks; // , back;
  entireScreen.subdiv(16 * 16, blocks);

  { // first we render a rough scene
    boost::thread_group threads;
    for (size_t i = 0; i < nP; ++i) {
      threads.create_thread(Task(i, nP, blocks, *this, Rough));
    }

    threads.join_all();
    std::cout << "Rough done..." << std::endl;
  }

  blocks.clear();
  entireScreen.subdiv(8 * 8, blocks);

  { // render good scene
    boost::thread_group threads;
    for (size_t i = 0; i < nP; ++i) {
      threads.create_thread(Task(i, nP, blocks, *this, Nice));
    }

    threads.join_all();
  }

  const auto td =
      time_period(start_time, microsec_clock::local_time()).length();
  std::cout << "Took " << td << std::endl;
}

std::ostream& operator<<(std::ostream& o, Scene const& s) {
  o << "Scene: {\n";
  o << s.m_camera << '\n';
  o << "Background: " << s.m_background << '\n';
  o << "Primitives: {\n";
  o << *s.m_manager;
  o << "}\n}";
  return o;
}
