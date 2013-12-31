#ifndef RAY_SPHERE_H
#define RAY_SPHERE_H

#include "common.h"
#include "geometry.h"
#include "intersection.h"
#include "primitive.h"
#include "ray.h"
#include "light.h"

/**
 * @ingroup Primitives
 * A sphere.
 */
class Sphere : public Primitive {
public: /* Methods: */

  Sphere(const Point& c, floating r)
    : m_center(c)
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

  floating getLeftExtreme(size_t axis) const {
    return m_center[axis] - m_radius;
  }

  floating getRightExtreme(size_t axis) const {
    return m_center[axis] + m_radius;
  }

  const Colour getColourAtIntersection(const Point& point, const Texture* texture) const {
    const auto D = - this->normal(point);
    const auto u = 0.5 + atan2(D.z, D.x) / (2 * M_PI);
    const auto v = 0.5 - asin(D.y) / M_PI;
    return texture->getTexel(u, v);
  }

protected: /* Fields: */
  const Point    m_center;    ///< Center of the sphere
  const floating m_radius;    ///< Radius of the sphere
  const floating m_sqrradius; ///< Square of the spheres radius
};

#endif

