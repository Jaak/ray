#ifndef RAY_RENDERER_H
#define RAY_RENDERER_H

#include <memory>

class Colour;
class Ray;
class Scene;
class Framebuffer;

class Renderer {
public: /* Methods: */

  Renderer (const Scene& scene)
    : m_scene (scene)
  { }

  virtual ~Renderer () { }

  virtual void render (Framebuffer& buf, size_t iter);

  virtual std::unique_ptr<Renderer> clone () const = 0;

  const Scene& scene () const { return m_scene; }

protected:

  virtual Colour render (Ray ray);

protected: /* Fields: */
  const Scene& m_scene;
};

#endif
