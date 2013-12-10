#ifndef RAY_SPOTLIGHT_H
#define RAY_SPOTLIGHT_H

#include "light.h"
#include "sphere_cap.h"
#include "random.h"

/// Directional light source.
class Spotlight : public SphereCap, public Light {
public: /* Methods: */

  Spotlight(const Point& p, const float r, const Vector& d, const float a, const Colour& c)
    : SphereCap(p, r, d, a, true)
    , Light(c)
    {}

  Spotlight(const Point& p, const float r, const Vector& d, const float a)
    : SphereCap(p, r, d, a, true)
    , Light(Colour(1, 1, 1))
    {}

  const Primitive* prim () const { return this; }




  Ray sample () const {
    
    auto H = Vector {0, 0, 0};
    while (true) {
      H = rngHemisphere ();
      if (H.z >= cos (m_angle))
        break;
    }

    const auto D = Frame { m_direction }.toWorld (H);
    const auto P = center () + radius () * D;

    return {P.nudgePoint (P), D};
  }

  floating lightPA () const {
    floating h = m_radius * (1.0 - cos(m_angle));
    floating area = 2 * M_PI * m_radius * h;
    return 1.0 / area;
  }

};

#endif
