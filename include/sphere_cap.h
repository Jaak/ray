#ifndef RAY_SPHERE_CAP_H
#define RAY_SPHERE_CAP_H

#include "common.h"
#include "geometry.h"
#include "intersection.h"
#include "primitive.h"
#include "ray.h"
#include "light.h"
#include "sphere.h"

#include <iostream>


class SphereCap : public Sphere {
public: /* Methods: */

  SphereCap(const Point& c, floating r, const Vector& d, floating a, bool is_light = false)
    : Sphere (c, r, is_light)
    , m_direction(normalised(d))
    , m_angle(a)
  { }

  void intersect(const Ray& ray, Intersection& intr) const {
    const Vector tmp = ray.origin() - m_center;
    const floating p = tmp.dot(ray.dir());
    const floating q = tmp.sqrlength() - m_sqrradius;
    floating d = p*p - q;

    if (d < 0) {
      return;
    }

    // angle between ray and cap direction
    floating a = acos((-1*ray.dir()).dot(m_direction) / ray.dir().length()) * 180.0 / M_PI;

    // ray intersects the sphere, but not the cap
    if (a > m_angle) {
      return;
    }

    d = std::sqrt(d);
    intr.update (ray, this, -p + d);
    intr.update (ray, this, -p - d);
  }

  void output(std::ostream& o) const {
    o << "SphereCap {";
    o << "center = " << m_center << ',';
    o << "radius = " << m_radius << ',';
    o << "cap direction = " << m_direction << ',';
    o << "angle = " << m_angle;
    o << '}';
  }

protected: /* Fields: */
  const Vector m_direction;
  const floating m_angle;
};

#endif

