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

  PointLight(const Point& p, const Colour& c) : Sphere(p, 0.01, true), Light(c) {}

  PointLight(const Point& p) : Sphere(p, 0.01, true), Light(Colour(1, 1, 1)) {}

  const Primitive* prim () const { return this; }

  Ray sample () const {
    return { center(), rngSphere ()};
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

    SphereLight(const Point& p, floating r, const Colour& c) : Sphere(p, r, true), Light(c) {}

    SphereLight(const Point& p, floating r) : Sphere(p, r, true), Light(Colour(1, 1, 1)) {}

    const Primitive* prim () const { return this; }

    Ray sample () const {
        const auto normal = rngSphere ();
        const auto point = center () + radius ()*normal;
        const auto dir = rngHemisphereVector (normal);
        return { point.nudgePoint (normal), dir };
    }
};

#endif
