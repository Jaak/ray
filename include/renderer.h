#ifndef RAY_RENDERER_H
#define RAY_RENDERER_H

class Colour;
class Ray;
class Scene;

class Renderer {
public: /* Methods: */

  Renderer (const Scene& scene)
    : m_scene (scene)
  { }

  virtual ~Renderer () { }

  virtual Colour render (Ray ray) = 0;

  const Scene& scene () const { return m_scene; }

protected: /* Fields: */
  const Scene& m_scene;
};

#endif
