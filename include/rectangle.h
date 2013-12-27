#ifndef RAY_RECTANGE_H
#define RAY_RECTANGE_H

#include "primitive.h"
#include "ray.h"
#include "intersection.h"

class Rectangle : public Primitive {
private: /* Methods: */

  static inline floating det(floating x1, floating x2, floating x3, floating y1,
                             floating y2, floating y3, floating z1, floating z2,
                             floating z3) {
    return x1 * (y2 * z3 - y3 * z2) -
           x2 * (y1 * z3 - y3 * z1) +
           x3 * (y1 * z2 - y2 * z1);
  }

  static inline floating det(const Vector& u, const Vector& v, const Vector& w) {
    return det (u[0], u[1], u[2], v[0], v[1], v[2], w[0], w[1], w[3]);
  }

public:

  Rectangle(const Point& point, const Vector& u, const Vector& v)
    : m_point { point }
    , m_u { u }
    , m_v { v }
    , m_normal { normalised (u.cross (v)) }
  { }

  void intersect(const Ray& ray, Intersection& intr) const {
    const auto D = ray.dir ();
    const auto pvec = D.cross (m_v);
    const auto det = m_u.dot (pvec);
    if (det == 0.0)
      return;

    const auto invDet = 1.0 / det;
    const auto tvec = ray.origin () - m_point;
    const auto u = tvec.dot (pvec) * invDet;
    if (u < 0.0 || u > 1.0)
      return;

    const auto qvec = tvec.cross (m_u);
    const auto v = D.dot (qvec) * invDet;
    if (v < 0.0 || v > 1.0)
      return;

    intr.update (ray, this, m_v.dot (qvec) * invDet);
  }

  Vector normal(const Point&) const { return m_normal; }

  floating getLeftExtreme(Axes axis) const {
    return
      fmin(m_point[axis],
      fmin(m_point[axis] + m_u[axis],
      fmin(m_point[axis] + m_v[axis],
      m_point[axis] + m_v[axis] + m_u[axis])));
  }

  floating getRightExtreme(Axes axis) const {
    return
      fmax(m_point[axis],
      fmax(m_point[axis] + m_u[axis],
      fmax(m_point[axis] + m_v[axis],
      m_point[axis] + m_v[axis] + m_u[axis])));
  }

protected:
  const Point m_point;
  const Vector m_u, m_v, m_normal;
};

#endif
