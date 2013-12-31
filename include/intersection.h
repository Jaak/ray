#ifndef RAY_INTERSECTION_H
#define RAY_INTERSECTION_H

#include "geometry.h"
#include "ray.h"

class Primitive;

/// Representation of intersection of ray and object.
class Intersection {
public: /* Methods: */

  Intersection() : m_primitive {nullptr} {}

  const Point& point() const { return m_point; }

  floating dist() const { return m_dist; }

  const Primitive* getPrimitive() const { return m_primitive; }

  void nullPrimitive() { m_primitive = nullptr; }

  bool hasIntersections() const { return m_primitive != nullptr; }

  void update(const Ray& ray, const Primitive* prim, floating dist) {
      if (dist > 0.0 && (!hasIntersections() || dist < this->dist())) {
          m_primitive = prim;
          m_dist = dist;
          m_point = ray.origin() + dist * ray.dir();
      }
}

private: /* Fields: */
  const Primitive* m_primitive; ///< Intersected primitive.
  Point            m_point;     ///< Point of intersection.
  floating         m_dist;      ///< Distance from origin.
};

#endif
