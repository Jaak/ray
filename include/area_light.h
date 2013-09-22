#ifndef RAY_AREA_LIGHT_H
#define RAY_AREA_LIGHT_H

#include "geometry.h"
#include "rectangle.h"
#include "random.h"
#include "light.h"

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

  Ray emit(Random& gen) const {
    const floating h = gen();
    const floating k = gen();
    const Point P = m_point + h * m_u + k * m_v;
    const Vector U = { gen() - 0.5, gen() - 0.5, gen() - 0.5 };
    const Vector V = normalised(m_normal + 2.0 * U);
    return { P + ray_epsilon * V, V };
  }
};

#endif
