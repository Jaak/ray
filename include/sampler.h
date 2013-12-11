#ifndef RAY_SAMPLER_H
#define RAY_SAMPLER_H

#include "common.h"

class Camera;
class Colour;
class Ray;
class Renderer;

class Sampler {
public: /* Methods: */

  Sampler ()
    : m_camera { nullptr }
    , m_renderer { nullptr }
  { }

  virtual ~Sampler () { }

  virtual Colour samplePixel (floating x, floating y) = 0;

  void setCamera (const Camera& cam) { m_camera = &cam; }
  void setRenderer (Renderer& rend) { m_renderer = &rend; }

protected: /* Methods: */

  const Camera& camera () const { return *m_camera; }
  const Renderer& renderer () const { return *m_renderer; }
  Renderer& renderer () { return *m_renderer; }

protected: /* Fields: */
  const Camera* m_camera;
  Renderer* m_renderer;
};

#endif
