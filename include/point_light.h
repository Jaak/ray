#ifndef RAY_POINT_LIGHT_H
#define RAY_POINT_LIGHT_H

#include "sphere.h"
#include "geometry.h"
#include "light.h"

/// Point light source.
class PointLight : public Sphere, public Light {
public: /* Methods: */

  PointLight(const Point& p, const Colour& c) : Sphere(p, epsilon), Light(c) {}

  PointLight(const Point& p) : Sphere(p, epsilon), Light(Colour(1, 1, 1)) {}

  LightType type(void) const { return POINT_LIGHT; }

  bool is_light(void) const { return true; }

  Ray emit(Random& gen) const {
    const Vector u = { gen() - 0.5, gen() - 0.5, gen() - 0.5 };
    const Vector v = normalised(u);
    const Point p = center() + 2.0 * ray_epsilon * v;
    return { p, v };
  }
};

#endif
