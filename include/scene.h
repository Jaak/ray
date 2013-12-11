#ifndef RAY_SCENE_H
#define RAY_SCENE_H

#include "camera.h"
#include "common.h"
#include "geometry.h"
#include "material.h"
#include "materials.h"
#include "surface.h"
#include "texture.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

class Block;
class Light;
class Pixel;
class Primitive;
class PrimitiveManager;
class Renderer;
class Sampler;
class SceneReader;

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

  void setRenderer(Renderer* r);

  void setSampler(Sampler* s);

  void setSceneReader(SceneReader* sr);

  void addPrimitive (const Primitive* prim);
  void addLight (Light*  light);
  const std::vector<Light* >& lights() const;
  void attachSurface(Surface* surface) { m_surfaces.emplace_back(surface); }
  const std::vector<std::unique_ptr<Surface>>& surfaces() const { return m_surfaces; }
  PrimitiveManager& manager() const { return *m_manager; }
  const Materials& materials () const { return m_materials; }
  Materials& materials () { return m_materials; }
  Camera& camera() { return m_camera; }
  const Camera& camera() const { return m_camera; }
  Sampler& sampler () { return *m_sampler; }
  const Sampler& sampler () const { return *m_sampler; }
  Renderer& renderer () { return *m_renderer; }
  const Renderer& renderer () const { return *m_renderer; }
  void setBackground(const Colour& c);
  const Material& background() const;
  std::string getFname();
  friend std::ostream& operator << (std::ostream&, Scene const&);
  const Textures& textures () const { return m_textures; }
  Textures& textures () { return m_textures; }

private:
  Colour trace(size_t, Pixel**, int, int, int, int, int, int, int);

protected: /* Fields: */
  Camera                                m_camera;        ///< Camera of the scene.
  Materials                             m_materials;
  std::unique_ptr<PrimitiveManager>     m_manager;       ///< Primitive manager of the scene.
  std::unique_ptr<SceneReader>          m_scene_reader;  ///< Way to read scene from a source.
  std::vector<Light* >                  m_lights;        ///< Lights of the scene.
  material_index_t                      m_background;    ///< Background material.
  std::vector<std::unique_ptr<Surface>> m_surfaces;      ///< Output surfaces.
  Textures                              m_textures;
  std::unique_ptr<Renderer>             m_renderer;
  std::unique_ptr<Sampler>              m_sampler;
};

#endif

