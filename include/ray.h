#ifndef RAY_RAY_H
#define RAY_RAY_H

#include <iostream>
#include "geometry.h"

class Intersection;

class Ray {
public:

  Ray (const Point& o, const Vector& d)
    : m_origin { o }
    , m_dir { d }
  { }

  floating origin (Axes ax) const { return m_origin[ax]; }
  floating dir (Axes ax) const { return m_dir[ax]; }
  Point origin () const { return m_origin; }
  Vector dir () const { return m_dir; }

  Ray reflect (const Intersection& intr) const;
  Vector normal (const Intersection& intr) const;

  friend std::ostream& operator << (std::ostream& os, const Ray& r) {
    os << "Ray {" << r.origin () << "," << r.dir () << "}";
    return os;
  }

private: /* Fields: */
  const Point  m_origin;
  const Vector m_dir;
};

#endif

