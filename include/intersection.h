#ifndef RAY_INTERSECTION_H
#define RAY_INTERSECTION_H

#include "geometry.h"
#include <iostream>

class Primitive;
class Ray;

/// Representation of intersection of ray and object.
class Intersection {
public: /* Types: */

  enum class Type {
    EXTERNAL = 0, INTERNAL
  };

public: /* Methods: */

  Intersection() : m_primitive{ nullptr } {}

  void setPoint(const Point& p) { m_point = p; }
  const Point& point() const { return m_point; }
  void setDist(floating d) { m_dist = d; }
  floating dist() const { return m_dist; }
  const Primitive* getPrimitive() const { return m_primitive; }
  void setPrimitive(const Primitive* p) { m_primitive = p; }
  Type type() const { return m_type; }
  void setType(Type type) { m_type = type; }

  bool isInternal () const { return m_type == Type::INTERNAL; }
  bool isExternal () const { return m_type == Type::EXTERNAL; }

  void nullPrimitive() { m_primitive = nullptr; }

  bool hasIntersections() const { return m_primitive != nullptr; }

  void update(const Ray& ray, const Primitive* prim, floating dist, Type type);

  friend std::ostream& operator<<(std::ostream&, const Intersection&);

private: /* Fields: */
  const Primitive* m_primitive; ///< Intersected primitive.
  Point            m_point;     ///< Point of intersection.
  floating         m_dist;      ///< Distance from origin.
  Type             m_type;      ///
};

#endif
