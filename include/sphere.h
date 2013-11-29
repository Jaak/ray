#ifndef RAY_SPHERE_H
#define RAY_SPHERE_H

#include "common.h"
#include "geometry.h"
#include "intersection.h"
#include "primitive.h"
#include "ray.h"
#include "light.h"

#include <iostream>

/**
 * @ingroup Primitives
 * A sphere.
 */
class Sphere : public Primitive {
public: /* Methods: */

  Sphere(const Point& c, floating r, bool is_light = false)
    : Primitive (is_light)
    , m_center(c)
    , m_radius(r)
    , m_sqrradius(r*r)
  { }

  const Point& center() const { return m_center; }
  floating radius() const { return m_radius; }

  Vector normal(const Point& p) const {
      return normalised(p - m_center);
  }

  void intersect(const Ray& ray, Intersection& intr) const {
    const Vector tmp = ray.origin() - m_center;
    const floating p = tmp.dot(ray.dir());
    const floating q = tmp.sqrlength() - m_sqrradius;
    floating d = p*p - q;

    if (d < 0) {
      return;
    }

    d = std::sqrt(d);
    intr.update (ray, this, -p + d);
    intr.update (ray, this, -p - d);
  }

  floating getLeftExtreme(Axes axis) const {
    return m_center[axis] - m_radius;
  }

  floating getRightExtreme(Axes axis) const {
    return m_center[axis] + m_radius;
  }

  void output(std::ostream& o) const {
    o << "Sphere {";
    o << "center = " << m_center << ',';
    o << "radius = " << m_radius;
    o << '}';
  }

protected: /* Fields: */
  const Point    m_center;    ///< Center of the sphere
  const floating m_radius;    ///< Radius of the sphere
  const floating m_sqrradius; ///< Square of the spheres radius
};

#endif

