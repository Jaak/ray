#ifndef RAY_POINT_LIGHT_H
#define RAY_POINT_LIGHT_H

#include "geometry.h"
#include "light.h"
#include "random.h"
#include "sphere.h"

/// Point light source.
class PointLight : public Sphere, public Light {
public: /* Methods: */

  PointLight(const Point& p, const Colour& c) : Sphere(p, epsilon), Light(c) {}

  PointLight(const Point& p) : Sphere(p, epsilon), Light(Colour(1, 1, 1)) {}

  LightType type(void) const { return POINT_LIGHT; }

  bool is_light(void) const { return true; }

  Ray emit() const {
    const Vector u = { rng() - 0.5, rng() - 0.5, rng() - 0.5 };
    const Vector v = normalised(u);
    const Point p = center() + 2.0 * ray_epsilon * v;
    return { p, v };
  }
};

#endif
