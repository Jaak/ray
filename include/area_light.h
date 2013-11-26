#ifndef RAY_AREA_LIGHT_H
#define RAY_AREA_LIGHT_H

#include "geometry.h"
#include "light.h"
#include "random.h"
#include "rectangle.h"

class AreaLight : public Rectangle, public Light {
public: /* Methods: */

  AreaLight(const Rectangle& rect, const Colour& c)
    : Rectangle(rect), Light(c)
  { }

  LightType type () const {
    return AREA_LIGHT;
  }

  bool is_light() const {
    return true;
  }

  Point middle() const {
    return m_point + 0.5*m_u + 0.5*m_v;
  }

  const Primitive* prim () const {
      return this;
  }

  Ray sample() const {
    const floating h = rng();
    const floating k = rng();
    const Point P = m_point + h * m_u + k * m_v;
    const Vector U = { rng() - 0.5, rng() - 0.5, rng() - 0.5 };
    const Vector V = normalised(m_normal + 2.0 * U);
    return { P + ray_epsilon * V, V };
  }
};

#endif
