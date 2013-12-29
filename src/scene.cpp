#include "camera.h"
#include "common.h"
#include "framebuffer.h"
#include "light.h"
#include "parser.h"
#include "primitive_manager.h"
#include "renderer.h"
#include "scene.h"
#include "scene_reader.h"

#include <boost/date_time.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
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

std::string Scene::getFname() {return (*m_scene_reader).getFname();}

Scene::~Scene() { }

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

  {
      using namespace boost::posix_time;
      const auto start_time = microsec_clock::local_time();
      m_manager->init();
      const auto td = time_period(start_time, microsec_clock::local_time()).length();
      std::cout << "Building acceleration structure took " << td << std::endl << std::endl;
  }
}

void Scene::addPrimitive(const Primitive* p) { m_manager->addPrimitive(p); }

void Scene::addLight(Light* l) { m_lights.emplace_back (l); }

const std::vector<std::unique_ptr<Light>>& Scene::lights() const { return m_lights; }

void Scene::updatePixel (size_t x, size_t y, Colour c) const {
    for (auto& surface : m_surfaces) {
        surface->setPixel (y, x, c);
    }
}

void Scene::run() {
    using namespace boost::posix_time;
    const auto start_time = microsec_clock::local_time();
    const size_t nP = std::max(boost::thread::hardware_concurrency(), 1u);
    std::cout << "Rendering on " << nP << " threads." << std::endl;

    boost::mutex countMutex;
    boost::thread_group threads;
    size_t count = 0;
    bool done = false;
    const auto width = m_camera.width ();
    const auto height = m_camera.height ();
    Framebuffer frame = {width, height};

    // Spawn rendering threads
    for (size_t i = 0; i < nP; ++ i) {
        const auto renderFunc = [&, i]() {
            auto renderer = m_renderer->clone ();
            for (size_t j = i; j < m_samples; j += nP) {
                renderer->render (frame, j);
                frame.flushUpdates ();
                boost::mutex::scoped_lock scoped_lock (countMutex);
                ++ count;
            }
        };

       threads.create_thread (renderFunc);
    }

    // Spawn display thread
    const auto displayFunc = [this, &done, &count, &countMutex, &frame]() {
        while (! done) { // it's ok to use "done" in unsafe manner.
            size_t localCount = 0;
            {
                boost::mutex::scoped_lock scoped_lock (countMutex);
                localCount = count;
            }

            if (localCount > 0) {
                for (size_t x = 0; x < frame.width (); ++ x) {
                    for (size_t y = 0; y < frame.height (); ++ y) {
                        const auto c = frame.unsafeGetPixel (x, y);
                        updatePixel (x, y, c / localCount);
                    }
                }
            }

            boost::this_thread::sleep (boost::posix_time::seconds (1));
        }
    };

    boost::thread displayThread {displayFunc};
    threads.join_all();
    done = true;
    displayThread.join ();

    // m_manager->debugDrawOnFramebuffer (m_camera, frame);

    for (size_t x = 0; x < frame.width (); ++ x) {
        for (size_t y = 0; y < frame.height (); ++ y) {
            const auto c = frame.unsafeGetPixel (x, y);
            updatePixel (x, y, c / count);
        }
    }

    const auto td =
        time_period(start_time, microsec_clock::local_time()).length();
    std::cout << "Took " << td << std::endl;
}
