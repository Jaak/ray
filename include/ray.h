#ifndef RAY_RAY_H
#define RAY_RAY_H

#include "geometry.h"

class Intersection;

class Ray {
public:

  Ray (const Point& o, const Vector& d)
    : m_origin {o}
    , m_dir {d}
  { }

  floating origin (size_t ax) const { return m_origin[ax]; }
  floating dir (size_t ax) const { return m_dir[ax]; }
  Point origin () const { return m_origin; }
  Vector dir () const { return m_dir; }

  Ray reflect (const Intersection& intr) const;

private: /* Fields: */
  Point  m_origin;
  Vector m_dir;
};

#endif

