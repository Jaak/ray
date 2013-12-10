#ifndef RAY_POINT_LIGHT_H
#define RAY_POINT_LIGHT_H

#include "geometry.h"
#include "light.h"
#include "random.h"
#include "sphere.h"

/**
 * TODO: figure out why setting the radius to epsilon or
 * zero causes massive rendering errors.
 */

/**
 * Point lights are just SphereLight's with really small radius.
 * We are not reusing SphereLight-s class as we can provide more efficient
 * and accurate implementation for sampling.
 */
class PointLight : public Sphere, public Light {
public: /* Methods: */

  PointLight(const Point& p, const Colour& c = Colour { 1, 1, 1}, floating emission = 1.0)
    : Sphere(p, 0.01, true), Light(c, emission)
  {}

  const Primitive* prim () const { return this; }
  const Light* as_light () const { return this; }

  Ray sample () const {
    return { center(), rngSphere ()};
  }

  floating lightPA () const {
      return 1.0;
  }
};

/**
 * Spherical light does not emit directed light.
 * Light rays are sampled as follows:
 * 1) pick random point on the surface of the light source
 * 2) pick random direction on hemisphere pointing away from the picked point
 */
class SphereLight : public Sphere, public Light {
public: /* Methods: */

    SphereLight(const Point& p, floating r, const Colour& c = Colour{1,1,1}, floating emission = 1.0)
      : Sphere(p, r, true), Light(c, emission)
    {}

    const Primitive* prim () const { return this; }
    const Light* as_light () const { return this; }

    Ray sample () const {
        const auto normal = rngSphere ();
        const auto point = center () + radius ()*normal;
        const auto dir = rngHemisphereVector (normal);
        return { point.nudgePoint (dir), dir };
    }

    floating lightPA () const {
        if (radius () < epsilon)
            return 1.0;

        return 1.0 / (4.0 * M_PI * m_sqrradius);
    }
};

#endif
