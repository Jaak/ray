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
    const auto V = rngHemisphereVector (m_normal);
    return { P + ray_epsilon * m_normal, V };
  }

  floating lightPA () const {
      const auto area = m_u.length () * m_v.length ();
      if (area < epsilon)
          return 1.0;

      return 1.0 / area;
  }
};

#endif
