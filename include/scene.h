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
#include <memory>

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

  Scene();

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
  void setPrimitiveManager(PrimitiveManager* pm);

  void setSceneReader(SceneReader* sr);

  void addPrimitive (Primitive* prim);
  void addLight (const Light*  light);
  const std::vector<const Light* >& lights() const;
  void attachSurface(Surface* surface) { m_surfaces.emplace_back(surface); }
  const std::vector<std::unique_ptr<Surface>>& surfaces() const { return m_surfaces; }
  PrimitiveManager& manager() const { return *m_manager; }
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
  Camera                                m_camera;        ///< Camera of the scene.
  Materials                             m_materials;
  std::unique_ptr<PrimitiveManager>     m_manager;       ///< Primitive manager of the scene.
  std::unique_ptr<SceneReader>          m_scene_reader;  ///< Way to read scene from a source.
  std::vector<const Light* >            m_lights;        ///< Lights of the scene.
  Colour                                m_background;    ///< Background colour of the scene.
  std::vector<std::unique_ptr<Surface>> m_surfaces;      ///< Output surfaces.
};

#endif

