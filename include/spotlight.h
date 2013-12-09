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
    const auto point = center () + radius ()*m_direction;
    // TODO - need a better way to get a random ray in the right direction
    while (true) {
      const auto dir = rngHemisphereVector(m_direction);
      // angle between dir and m_direction
      floating a = acos(m_direction.dot(dir) / dir.length()) * 180.0 / M_PI;
      /*std::cout << "point: " << point << std::endl;
      std::cout << "m_direction: " << m_direction << std::endl;
      std::cout << "dir: " << dir << std::endl;
      std::cout << "m_angle: " << m_angle << std::endl;
      std::cout << "a: " << a << std::endl;*/
      if (a < m_angle) {
        return {point.nudgePoint (dir), dir};
      }
    }   
  }

  floating lightPA () const {
      return 1.0;
  }

};

#endif
