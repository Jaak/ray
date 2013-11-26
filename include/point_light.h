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

/// Point light source.
class PointLight : public Sphere, public Light {
public: /* Methods: */

  PointLight(const Point& p, const Colour& c) : Sphere(p, 0.01, true), Light(c) {}

  PointLight(const Point& p) : Sphere(p, 0.01, true), Light(Colour(1, 1, 1)) {}

  const Primitive* prim () const { return this; }

  Ray sample() const {
    return { center(), rngSphere ()};
  }
};

#endif
