#ifndef RAY_TRIANGLE_H
#define RAY_TRIANGLE_H

#include <iostream>

#include "primitive.h"
#include "intersection.h"
#include "geometry.h"
#include "ray.h"

/**
 * @ingroup Primitives
 * A triangle
 */
class Triangle : public Primitive {
public: /* Methods: */

  Triangle(const Point& a, const Point& b, const Point& c)
    : m_point { a, b, c }
    , m_normal { computNormal () }
  { }

  Vector normal(const Point&) const { return m_normal; }

  // http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-9-ray-triangle-intersection/m-ller-trumbore-algorithm/
  void intersect(const Ray& ray, Intersection& intr) const {
    const auto edge1 = m_point[2] - m_point[0];
    const auto edge2 = m_point[1] - m_point[0];
    const auto D = ray.dir ();
    const auto pvec = D.cross (edge2);
    const auto det = edge1.dot (pvec);
    if (det == 0.0)
      return;

    const auto invDet = 1.0 / det;
    const auto tvec = ray.origin () - m_point[0];
    const auto u = tvec.dot (pvec) * invDet;
    if (u < 0.0 || u > 1.0)
      return;

    const auto qvec = tvec.cross (edge1);
    const auto v = D.dot (qvec) * invDet;
    if (v < 0.0 || u + v > 1.0)
      return;

    intr.update (ray, this, edge2.dot (qvec) * invDet);
  }

  floating getLeftExtreme(Axes axis) const {
    return fmin(fmin(m_point[0][axis], m_point[1][axis]), m_point[2][axis]);
  }

  floating getRightExtreme(Axes axis) const {
    return fmax(fmax(m_point[0][axis], m_point[1][axis]), m_point[2][axis]);
  }

  const Colour getColourAtIntersection(const Point& point, const Texture* texture) const {
    // http://gamedev.stackexchange.com/a/23745

    // barycentric coords of the triangle
    floating bu, bv, bw;

    // texture coords
    floating tu, tv;

    Vector v0 = m_point[1] - m_point[0];
    Vector v1 = m_point[2] - m_point[0];
    Vector v2 = point - m_point[0];
    floating d00 = v0.dot(v0);
    floating d01 = v0.dot(v1);
    floating d11 = v1.dot(v1);
    floating d20 = v2.dot(v0);
    floating d21 = v2.dot(v1);
    floating denom =  d00 * d11 - d01 * d01;
    bv = (d11 * d20 - d01 * d21) / denom;
    bw = (d00 * d21 - d01 * d20) / denom;
    bu = 1.0 - bv - bw;
    tu = bu * m_point[0].u + bv * m_point[1].u + bw * m_point[2].u;
    tv = bu * m_point[0].v + bv * m_point[1].v + bw * m_point[2].v;

    return texture->getTexel(tu, tv);
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

protected: /* Fields: */
  const Point     m_point[3]; ///< The points of the triangle
  const Vector    m_normal;   ///< Normal of the triangle
};

#endif

