#ifndef RAY_ITRIANGLE_H
#define RAY_ITRIANGLE_H

#include <iostream>

#include "primitive.h"
#include "geometry.h"

/**
 * A vertex.
 * Used as a helper in ITriangle.
 */
struct Vertex {
  Point m_pos;
  Vector m_normal;

  Vertex() {}

  Vertex(Point const& p, Vector const& n) : m_pos(p), m_normal(n) {}
};

/**
 * @ingroup Primitives
 *
 * %Triangle which has normal for each vertex.
 *
 * Following code is directly ripped.
 * It implements barycentryc coordinate intersection test and it is darn fast.
 *
 * My only excuse is that i wanted to render a smooth teapot.
 */
class ITriangle : public Primitive {
public: /* Methods: */

  ITriangle(const Point& p1, const Point& p2, const Point& p3, const Vector& n1,
            const Vector& n2, const Vector& n3) {
    m_verts[0] = Vertex(p1, n1);
    m_verts[1] = Vertex(p2, n2);
    m_verts[2] = Vertex(p3, n3);

    const Point A = m_verts[0].m_pos;
    const Point B = m_verts[1].m_pos;
    const Point C = m_verts[2].m_pos;
    const Vector c = B - A;
    const Vector b = C - A;

    const Vector N = b.cross(c);

    int u, v;

    if (fabs(N[Axes::X]) > fabs(N[Axes::Y])) {
      k = (fabs(N[Axes::X]) > fabs(N[Axes::Z]) ? 0 : 2);
    } else {
      k = (fabs(N[Axes::Y]) > fabs(N[Axes::Z]) ? 1 : 2);
    }

    u = (k + 1) % 3;
    v = (k + 2) % 3;

    const floating krec = 1.0 / N[k];
    nu = N[u] * krec;
    nv = N[v] * krec;
    nd = N.dot(A) * krec;

    const floating reci = 1.0 / (b[u] * c[v] - b[v] * c[u]);
    bnu = b[u] * reci;
    bnv = -b[v] * reci;
    cnu = c[v] * reci;
    cnv = -c[u] * reci;
  }

  Vector normal(const Point& pos) const {
      const Point A = m_verts[0].m_pos;
      const int ku = (k + 1) % 3, kv = (k + 2) % 3;
      const floating hu = pos[ku] - A[ku];
      const floating hv = pos[kv] - A[kv];
      const auto u = hv * bnu + hu * bnv;
      const auto v = hu * cnu + hv * cnv;
      return normalised(m_verts[0].m_normal +
              u * (m_verts[1].m_normal - m_verts[0].m_normal) +
              v * (m_verts[2].m_normal - m_verts[0].m_normal));
  }

  void intersect (const Ray& ray, Intersection& intr) const {
    const Point O = ray.origin();
    const Point A = m_verts[0].m_pos;
    const Vector D = ray.dir();
    const int ku = (k + 1) % 3, kv = (k + 2) % 3;
    const floating lnd =  1.0 / (D[k] + nu * D[ku] + nv * D[kv]);
    const floating t = (nd - O[k] - nu * O[ku] - nv * O[kv]) * lnd;
    const floating hu = O[ku] + t * D[ku] - A[ku];
    const floating hv = O[kv] + t * D[kv] - A[kv];

    const auto u = hv * bnu + hu * bnv;
    const auto v = hu * cnu + hv * cnv;

    if (u < 0 || v < 0 || u + v > 1)
      return;

    // TODO: side of the intersection matters
    intr.update (ray, this, t);
  }

  floating getLeftExtreme(Axes axis) const {
    return fmin(fmin(m_verts[0].m_pos[axis], m_verts[1].m_pos[axis]),
                m_verts[2].m_pos[axis]);
  }

  floating getRightExtreme(Axes axis) const {
    return fmax(fmax(m_verts[0].m_pos[axis], m_verts[1].m_pos[axis]),
                m_verts[2].m_pos[axis]);
  }

  const Colour getColourAtIntersection(const Point& point, const Texture* texture) const {
    const Point A = m_verts[0].m_pos;
    const int ku = (k + 1) % 3, kv = (k + 2) % 3;
    const floating hu = point[ku] - A[ku];
    const floating hv = point[kv] - A[kv];
    const auto u = hv * bnu + hu * bnv;
    const auto v = hu * cnu + hv * cnv;

    const auto tu = m_verts[0].m_pos.u + u*(m_verts[1].m_pos.u - m_verts[0].m_pos.u) + v * (m_verts[2].m_pos.u - m_verts[0].m_pos.u);
    const auto tv = m_verts[0].m_pos.v + u*(m_verts[1].m_pos.v - m_verts[0].m_pos.v) + v * (m_verts[2].m_pos.v - m_verts[0].m_pos.v);
    return texture->getTexel(tu, tv);
  }

private: /* Fields: */
  Vertex m_verts[3];
  floating nu, nv, nd;
  floating bnu, bnv;
  floating cnu, cnv;
  int k;
};

#endif
