#ifndef RAY_SCENE_H
#define RAY_SCENE_H

#include "common.h"
#include "camera.h"
#include "geometry.h"
#include "surface.h"
#include "materials.h"

#include <vector>
#include <string>
#include <cassert>

class Pixel;
class Block;
class Light;
class SceneReader;
class Primitive;
class PrimitiveManager;

/// Representation of scene.
class Scene {
  friend class Block;
public: /* Methods: */

  Scene()
    : m_camera (*this)
    , m_manager (nullptr)
    , m_scene_reader (nullptr)
    , m_background (0, 0, 0)
    , m_surface (nullptr)
  { }

  ~Scene();

  /**
   * Does the real raytracing.
   * Image is generated.
   */
  void run ();

  /**
   * Initialises scene for raytracing.
   * Complete destruction of our spacetime will ensure
   * if this method is not called before raytracing begins.
   *
   * @todo Throw logic exceptions from here.
   */
  void init ();

  /**
   * Sets scenes primitive manager.
   * Given PrimitiveManager will be deallocated after
   * Scene object is destroyed.
   */
  void setPrimitiveManager(PrimitiveManager* pm) {
    assert(m_manager == nullptr);
    m_manager = pm;
  }

  void setSceneReader(SceneReader* sr) {
    assert(m_scene_reader == nullptr);
    m_scene_reader = sr;
  }

  void addPrimitive (Primitive* prim);
  void addLight (const Light*  light);

  const std::vector<const Light* >& lights() const;

  void attachSurface(Surface* surface) { m_surface = surface; }
  Surface* surface() const { return m_surface; }

  PrimitiveManager* manager() const { return m_manager; }
  const Materials& materials () const { return m_materials; }
  Materials& materials () { return m_materials; }

  Camera& camera() { return m_camera; }
  const Camera& camera() const { return m_camera; }

  void setBackground(const Colour& c) { m_background = c; }
  const Colour& background() const { return m_background; }

  friend std::ostream& operator << (std::ostream&, Scene const&);

private:
  Colour trace(size_t, Pixel**, int, int, int, int, int, int, int);

protected: /* Fields: */
  Camera                      m_camera;        ///< Camera of the scene.
  Materials                   m_materials;
  PrimitiveManager*           m_manager;       ///< Primitive manager of the scene.
  SceneReader*                m_scene_reader;  ///< Way to read scene from a source.
  std::vector<const Light* >  m_lights;        ///< Lights of the scene.
  Colour                      m_background;    ///< Background colour of the scene.
  Surface*                    m_surface;       ///< The output image.
};

#endif

