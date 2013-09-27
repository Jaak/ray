#include "common.h"
#include "camera.h"
#include "scene.h"
#include "parser.h"
#include "raytracer.h"
#include "pixel.h"
#include "scene_reader.h"

#include <string>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

Scene::Scene()
  : m_background (0, 0, 0)
{ }

void Scene::setPrimitiveManager(PrimitiveManager* pm) {
  m_manager = std::unique_ptr<PrimitiveManager>(pm);
}

void Scene::setSceneReader(SceneReader* sr) {
  m_scene_reader = std::unique_ptr<SceneReader>(sr);
}

Scene::~Scene() { }

/**
 * Recursively shoots rays where needed.
 * @param current recursion depth.
 * @param subpixel block, used to cache already traced rays.
 * @param w,h position of ray on screen.
 * @param x1,y1 lover left
 * @param x2,y2 upper right
 * @param size width/height of subpixel block
 * @return gathered colour.
 */
Colour Scene::trace(size_t depth, Pixel** block, int w, int h, int x1, int y1,
                    int x2, int y2, int size) {
  Colour c[4];

  const floating isize = 1.0 / size;
  const floating dx[2] = { x1 * isize - 0.5, x2 * isize - 0.5 };
  const floating dy[2] = { y1 * isize - 0.5, y2 * isize - 0.5 };

  /**
   * Trace 4 corners of subpixel block.
   */
  if (!block[y1][x1].active()) {
    c[0] = Raytracer (*this)(m_camera.spawnRay(h + dy[0], w + dx[0]));
    block[y1][x1].activate();
    block[y1][x1].setColour(c[0]);
  } else {
    c[0] = block[y1][x1].colour();
  }

  if (!block[y2][x1].active()) {
    c[1] = Raytracer (*this)(m_camera.spawnRay(h + dy[1], w + dx[0]));
    block[y2][x1].activate();
    block[y2][x1].setColour(c[1]);
  } else {
    c[1] = block[y2][x1].colour();
  }

  if (!block[y1][x2].active()) {
    c[2] = Raytracer (*this)(m_camera.spawnRay(h + dy[0], w + dx[1]));
    block[y1][x2].activate();
    block[y1][x2].setColour(c[2]);
  } else {
    c[2] = block[y1][x2].colour();
  }

  if (!block[y2][x2].active()) {
    c[3] = Raytracer (*this)(m_camera.spawnRay(h + dy[1], w + dx[1]));
    block[y2][x2].activate();
    block[y2][x2].setColour(c[3]);
  } else {
    c[3] = block[y2][x2].colour();
  }

  if (depth <= AA_MAX_DEPTH) {
    // check if we need to go deeper
    for (int i = 0; i < 4; ++i) {
      for (int j = i + 1; j < 4; ++j) {
        if (!c[i].close(c[j])) {
          goto refine;
        }
      }
    }

    if (false) {
    refine:
      const int xm = (x1 + x2) / 2;
      const int ym = (y1 + y2) / 2;

      ++depth;

      // trace 4 more blocks
      c[0] = trace(depth, block, w, h, x1, y1, xm, ym, size);
      c[1] = trace(depth, block, w, h, x1, ym, xm, y2, size);
      c[2] = trace(depth, block, w, h, xm, y1, x2, ym, size);
      c[3] = trace(depth, block, w, h, xm, ym, x2, y2, size);
    }
  }

  // average of corner subpixels
  return (c[0] + c[1] + c[2] + c[3]) / 4.0;
}

#if MULTISAMPLE
void Scene::run() {
  Pixel** block;
  Pixel* cur;
  Pixel* prev;
  int sub_size = 1 << AA_MAX_DEPTH;
  int linebuf_size = sub_size * m_surface->width() + 1;

  // init
  block = new Pixel* [sub_size + 1];

  for (int i = 0; i <= sub_size; ++i) {
    block[i] = new Pixel[sub_size + 1];
  }

  cur = new Pixel[linebuf_size];
  prev = new Pixel[linebuf_size];

  for (int h = 0; h < m_surface->height(); ++h) {
    for (int i = 0; i < linebuf_size; ++i) {
      cur[i].reset();
    }

    for (int i = 0; i <= sub_size; ++i) {
      block[i][0].reset();
    }

    for (int w = 0; w < m_surface->width(); ++w) {
      for (int i = 1; i <= sub_size; ++i) {
        for (int j = 1; j <= sub_size; ++j) {
          block[i][j].reset();
        }
      }

      for (int i = 0, j = w * sub_size; j <= (w + 1) * sub_size; ++i, ++j) {
        block[0][i] = prev[j];
      }

      m_surface.setPixel(
          h, w, trace(1, block, w, h, 0, 0, sub_size, sub_size, sub_size));

      for (int i = 0, j = w * sub_size; j <= (w + 1) * sub_size; ++i, ++j) {
        prev[j] = block[0][i];
        cur[j] = block[sub_size][i];
      }

      for (int i = 0; i <= sub_size; ++i) {
        block[i][0] = block[i][sub_size];
      }
    }

    Pixel* const t = cur;
    cur = prev;
    prev = t;

    if ((h + 1) % (m_surface->height() / 20) == 0) {
      std::cout << '.';
      std::cout.flush();
    }
  }

  // free
  for (int i = 0; i < sub_size + 1; ++i) {
    delete[] block[i];
  }

  delete[] block;
  delete[] cur;
  delete[] prev;
}

#else

class Block {
public: /* Methods: */

  Block(int y0, int y1, int x0, int x1)
      : m_y0(y0), m_y1(y1), m_x0(x0), m_x1(x1) {}

  void renderRough(Scene& scene) {
    Colour c = Raytracer(scene)(scene.m_camera.spawnRay(m_y0, m_x0));
    for (int h = m_y0; h < m_y1; ++h) {
      for (int w = m_x0; w < m_x1; ++w) {
        for (auto& surface : scene.surfaces()) {
          surface->setPixel(h, w, c);
        }
      }
    }
  }

  void renderNice(Scene& scene) {
    for (int h = m_y0; h < m_y1; ++h) {
      for (int w = m_x0; w < m_x1; ++w) {
        for (auto& surface : scene.surfaces()) {
          surface->setPixel(h, w,
            Raytracer(scene)(scene.m_camera.spawnRay(h, w)));
        }
      }
    }
  }

  bool subdivide(std::vector<Block>& blocks) const {
    if ((m_y1 - m_y0) * (m_x1 - m_x0) < 2) {
      return false;
    }

    int ym = m_y0 + (m_y1 - m_y0) / 2;
    int xm = m_x0 + (m_x1 - m_x0) / 2;

    blocks.emplace_back(m_y0, ym, m_x0, xm);
    blocks.emplace_back(ym, m_y1, m_x0, xm);
    blocks.emplace_back(m_y0, ym, xm, m_x1);
    blocks.emplace_back(ym, m_y1, xm, m_x1);
    return true;
  }

private: /* Fields: */
  int m_y0, m_y1, m_x0, m_x1;
};

enum TaskType {
  Rough = 0,
  Nice
};

class Task {
public:
  Task(int i, int nP, std::vector<Block>& blocks, Scene& scene, TaskType type)
      : m_i(i), m_nP(nP), m_blocks(blocks), m_scene(scene), m_type(type) {}

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
  const int              m_i;
  const int              m_nP;
  std::vector<Block>&    m_blocks;
  Scene&                 m_scene;
  const TaskType         m_type;
};

void swap(std::vector<Block>& front, std::vector<Block>& back) {
  int end = front.size();

  back.clear();

  for (int i = 0; i < end; ++i) {
    front[i].subdivide(back);
  }

  front.clear();
  std::swap(front, back);
}

void Scene::run() {
  int nP;

  using namespace boost::posix_time;

  boost::thread_group threads;
  ptime start_time = microsec_clock::local_time();

  nP = std::max(boost::thread::hardware_concurrency(), 1u);

  std::cout << "Running on " << nP << " threads..." << std::endl;

  std::vector<Block> front;
  std::vector<Block> back;

  front.emplace_back(0, m_camera.height(), 0, m_camera.width());

  for (int i = 0; i < 6; ++i)
    swap(front, back);

  // first we render a rough scene
  for (int i = 0; i < nP; ++i) {
    threads.create_thread(Task(i, nP, front, *this, Rough));
  }

  threads.join_all();
  std::cout << "Rough done..." << std::endl;

  swap(front, back);

  for (int i = 0; i < nP; ++i) {
    threads.create_thread(Task(i, nP, front, *this, Nice));
  }

  threads.join_all();
  time_duration td =
      time_period(start_time, microsec_clock::local_time()).length();
  std::cout << "Took " << td << std::endl;
}
#endif

void Scene::init() {
  m_scene_reader->init(*this);
  for (auto& surface : m_surfaces) {
    surface->init();
  }

  m_camera.init();
  m_manager->init();
}

void Scene::addPrimitive(Primitive* p) { m_manager->addPrimitive(p); }

void Scene::addLight(const Light* l) { m_lights.push_back(l); }

const std::vector<const Light*>& Scene::lights() const { return m_lights; }

std::ostream& operator<<(std::ostream& o, Scene const& s) {
  o << "Scene: {\n";
  o << s.m_camera << '\n';
  o << "Background: " << s.m_background << '\n';
  o << "Primitives: {\n";
  o << *s.m_manager;
  o << "}\n}";
  return o;
}
