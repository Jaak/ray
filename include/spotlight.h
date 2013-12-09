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
      //printf("H.z=%f\n", H.z);
      if (H.z <= cos (m_angle * (M_PI / 180.0)))
        break;
    }

    const auto D = Frame { m_direction }.toWorld (H);
    const auto P = center () + radius () * D;
    
    //printf("m_angle %f\n", m_angle);
    //printf("angle %f\n", getAngle(D, m_direction));
    //printf("nAngle %f\n", getAngleN(D, m_direction));

    return {P.nudgePoint (P), D};

    /*
    // TODO - need a better way to get a random ray in the right direction
    while (true) {
      //const auto dir = rngHemisphereVector(m_direction);
      const auto dir = rngSphere();
      // angle between dir and m_direction
      floating a = getAngleN(m_direction, dir);
      if (a < m_angle) {
        return {point.nudgePoint (dir), dir};
      }
    } */ 

  }

  floating lightPA () const {
      return 1.0;
  }

};

#endif
