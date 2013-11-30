#ifndef RAY_INTERSECTION_H
#define RAY_INTERSECTION_H

#include "geometry.h"
#include <iostream>

class Primitive;
class Ray;

/// Representation of intersection of ray and object.
class Intersection {
public: /* Methods: */

  Intersection() : m_primitive { nullptr } {}

  const Point& point() const { return m_point; }

  floating dist() const { return m_dist; }

  const Primitive* getPrimitive() const { return m_primitive; }

  void nullPrimitive() { m_primitive = nullptr; }

  bool hasIntersections() const { return m_primitive != nullptr; }

  void update(const Ray& ray, const Primitive* prim, floating dist);

  friend std::ostream& operator<<(std::ostream&, const Intersection&);

private: /* Fields: */
  const Primitive* m_primitive; ///< Intersected primitive.
  Point            m_point;     ///< Point of intersection.
  floating         m_dist;      ///< Distance from origin.
};

#endif
