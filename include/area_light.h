#ifndef RAY_AREA_LIGHT_H
#define RAY_AREA_LIGHT_H

#include "geometry.h"
#include "light.h"
#include "random.h"
#include "rectangle.h"

class AreaLight : public Rectangle, public Light {
public: /* Methods: */

  AreaLight (Point p, Vector u, Vector v, Colour c)
    : Rectangle { p, u, v, true }
    , Light { c }
  { }

  AreaLight(const Rectangle& rect, const Colour& c)
    : Rectangle(rect), Light(c)
  { }

  const Primitive* prim () const {
      return this;
  }

  Ray sample() const {
    const auto h = rng();
    const auto k = rng();
    const auto P = m_point + h * m_u + k * m_v;
    auto V = Vector { };
    while (true) {
      V = 2.0 * Vector { rng () - 0.5, rng () - 0.5, rng () - 0.5 };
      if (V.sqrlength () > 1.0) continue;
      if (m_normal.dot (V) >= epsilon) {
        V = normalised (V);
        break;
      }
    }
    return { P + ray_epsilon * V, V };
  }
};

#endif
