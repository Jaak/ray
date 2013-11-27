#ifndef RAY_TRIANGLE_H
#define RAY_TRIANGLE_H

#include <iostream>

#include "primitive.h"
#include "intersection.h"
#include "geometry.h"

/**
 * @ingroup Primitives
 * A triangle
 */
class Triangle : public Primitive {
public: /* Methods: */

  Triangle(const Point& a, const Point& b, const Point& c)
    : m_point { a, b, c }
    , m_normal { computNormal () }
    , m_d { computeD () }
  { }

  Vector normal(const Point&) const { return m_normal; }

  void intersect(const Ray& ray, Intersection& intr) const {
    const Point o = ray.origin();
    const Vector vs[] = { m_point[0] - o, m_point[1] - o, m_point[2] - o };
    floating a = m_normal.dot(ray.dir());

    if (almost_zero(a)) {
      return;
    }

    a = -(m_normal.dot(o) + m_d) / a;

    const Vector pmo = a * ray.dir();
    if (vs[1].cross(vs[0]).dot(pmo) < 0 ||
        vs[2].cross(vs[1]).dot(pmo) < 0 ||
        vs[0].cross(vs[2]).dot(pmo) < 0) {
      return;
    }

    // TODO: side of the intersection matters!
    intr.update(ray, this, a, Intersection::Type::EXTERNAL);
  }

  floating getLeftExtreme(Axes axis) const {
    return fmin(fmin(m_point[0][axis], m_point[1][axis]), m_point[2][axis]);
  }

  floating getRightExtreme(Axes axis) const {
    return fmax(fmax(m_point[0][axis], m_point[1][axis]), m_point[2][axis]);
  }

  void output(std::ostream& o) const {
    o << "Triangle {";
    o << m_point[0] << ',';
    o << m_point[1] << ',';
    o << m_point[2] << ',';
    o << '}';
  }

private: /* Methods: */

  Vector computNormal() const {
    return normalised((m_point[1] - m_point[0]).cross(m_point[2] - m_point[0]));
  }

  floating computeD () const {
    return -m_normal.dot(m_point[0]);
  }

private: /* Fields: */ 
  const Point     m_point[3]; ///< The points of the triangle
  const Vector    m_normal;   ///< Normal of the triangle
  const floating  m_d;        ///< With m_normal represents the plane of the triangle.
};

#endif

