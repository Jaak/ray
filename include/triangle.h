#ifndef RAY_TRIANGLE_H
#define RAY_TRIANGLE_H

#include "geometry.h"
#include "intersection.h"
#include "primitive.h"
#include "ray.h"

/**
 * Policies that can provide normals and uv-coordinates
 */

struct NoNormalPolicy {
    static constexpr bool HasNormal = false;
};

struct NoUVPolicy {
    static constexpr bool HasUV = false;
};

struct HasNormalPolicy {
    Vector normal;

    static constexpr bool HasNormal = true;
};

struct HasUVPolicy {
    floating u;
    floating v;

    static constexpr bool HasUV = true;
};

/**
 * Polygon vertices may have specified normals and/or uv-coordinates.
 */

template <typename NormalPolicy, typename UVPolicy>
struct Vertex : public NormalPolicy, public UVPolicy {
    Point position;

    Vertex (Point position)
        : position (position)
    { }

    using NormalPolicy::HasNormal;
    using UVPolicy::HasUV;
};

Vertex<NoNormalPolicy, NoUVPolicy> make_vertex (Point pos) {
    return {pos};
}

Vertex<NoNormalPolicy, HasUVPolicy> make_vertex (Point pos, floating u, floating v) {
    auto result = Vertex<NoNormalPolicy, HasUVPolicy>{pos};
    result.u = u;
    result.v = v;
    return result;
}

Vertex<HasNormalPolicy, NoUVPolicy> make_vertex (Point pos, Vector normal) {
    auto result = Vertex<HasNormalPolicy, NoUVPolicy>{pos};
    result.normal = normal;
    return result;
}

Vertex<HasNormalPolicy, HasUVPolicy> make_vertex (Point pos, Vector normal, floating u, floating v) {
    auto result = Vertex<HasNormalPolicy, HasUVPolicy>{pos};
    result.normal = normal;
    result.u = u;
    result.v = v;
    return result;
}

/**
 * This CRTP class provides triangle with normals.
 */

// TODO: i'm quite sure we don't actually have to store normal specifically and
// it can be reconstructed. Revisit if we start having storage issues.
template <template <typename, typename> class Derived, typename NormalPolicy,
         typename UVPolicy>
class TriangleNormalCRTP {
private: /* Types: */
    using DerivedClass = Derived<NormalPolicy, UVPolicy>;
    using VertexType = Vertex<NormalPolicy, UVPolicy>;
public: /* Methods: */

    TriangleNormalCRTP (VertexType v1, VertexType v2, VertexType v3) {
        const auto b = v2.position - v1.position;
        const auto c = v3.position - v1.position;
        m_normal = normalised (b.cross(c));
    }

    inline Vector getNormal (Point) const {
        return m_normal;
    }

private: /* Fields: */
    Vector m_normal;
};

template <template <typename, typename> class Derived, typename UVPolicy>
class TriangleNormalCRTP<Derived, HasNormalPolicy, UVPolicy> {
private: /* Types: */
    using DerivedClass = Derived<HasNormalPolicy, UVPolicy>;
    using VertexType = Vertex<HasNormalPolicy, UVPolicy>;
public: /* Methods: */

    TriangleNormalCRTP (VertexType v1, VertexType v2, VertexType v3)
        : m_normals {v1.normal, v2.normal, v3.normal}
    { }

    inline Vector getNormal (Point pos) const {
        const auto& self = *static_cast<const DerivedClass*>(this);
        const int ku = (self.k + 1) % 3, kv = (self.k + 2) % 3;
        const floating hu = pos[ku] - self.points[0][ku];
        const floating hv = pos[kv] - self.points[0][kv];
        const auto u = hv * self.bnu + hu * self.bnv;
        const auto v = hu * self.cnu + hv * self.cnv;
        return normalised(m_normals[0] +
                u * (m_normals[1] - m_normals[0]) +
                v * (m_normals[2] - m_normals[0]));
    }

private: /* Fields: */
    Vector m_normals[3];
};

/**
 * This CRTP class provides triangle with uv-coordinates.
 */

template <template <typename, typename> class Derived, typename NormalPolicy, typename UVPolicy>
class TriangleUVCRTP {
private: /* Types: */
    using VertexType = Vertex<NormalPolicy, UVPolicy>;
public: /* Methods: */

    TriangleUVCRTP (VertexType, VertexType, VertexType) { }

    void getUV (Point, floating& tu, floating& tv) const {
        tu = 0.0; tv = 0.0;
    }
};

template <template <typename, typename> class Derived, typename NormalPolicy>
class TriangleUVCRTP<Derived, NormalPolicy, HasUVPolicy> {
private: /* Types: */
    using DerivedClass = Derived<NormalPolicy, HasUVPolicy>;
    using VertexType = Vertex<NormalPolicy, HasUVPolicy>;
public: /* Methods: */

    TriangleUVCRTP (VertexType v1, VertexType v2, VertexType v3)
        : m_uvs {{v1.u, v1.v}, {v2.u, v2.v}, {v3.u, v3.v}}
    { }

    void getUV (Point pos, floating& tu, floating& tv) const {
        const auto& self = *static_cast<const DerivedClass*>(this);
        const int ku = (self.k + 1) % 3, kv = (self.k + 2) % 3;
        const floating hu = pos[ku] - self.points[0][ku];
        const floating hv = pos[kv] - self.points[0][kv];
        const auto u = hv * self.bnu + hu * self.bnv;
        const auto v = hu * self.cnu + hv * self.cnv;
        tu = m_uvs[0][0] + u * (m_uvs[1][0] - m_uvs[0][0]) + v * (m_uvs[2][0] - m_uvs[0][0]);
        tv = m_uvs[0][1] + u * (m_uvs[1][1] - m_uvs[0][1]) + v * (m_uvs[2][1] - m_uvs[0][1]);
    }

private: /* Fields: */
    floating m_uvs[3][2];
};

template <typename NormalPolicy, typename UVPolicy>
class Triangle
    : public Primitive
    , TriangleNormalCRTP<Triangle, NormalPolicy, UVPolicy>
    , TriangleUVCRTP<Triangle, NormalPolicy, UVPolicy>
{
private: /* Types: */

    // In the following, if we just write Triangle that will be taken as alias to
    //  Triangle<NormalPolicy, UVPolicy>

    using VertexType = Vertex<NormalPolicy, UVPolicy>;
    using NormalBase = TriangleNormalCRTP<::Triangle, NormalPolicy, UVPolicy>;
    using UVBase = TriangleUVCRTP<::Triangle, NormalPolicy, UVPolicy>;

    // Woot for C++11 allowing for aliases in friend declarations
    friend NormalBase;
    friend UVBase;

public: /* Methods: */

    Triangle (VertexType v1, VertexType v2, VertexType v3)
        : NormalBase {v1, v2, v3}, UVBase {v1, v2, v3}
        , points {v1.position, v2.position, v3.position}
    {
        const auto A = v1.position;
        const auto B = v2.position;
        const auto C = v3.position;
        const auto c = B - A;
        const auto b = C - A;
        const auto N = b.cross(c);

        int u, v;
        if (fabs(N[0]) > fabs(N[1])) {
            k = (fabs(N[0]) > fabs(N[2]) ? 0 : 2);
        } else {
            k = (fabs(N[1]) > fabs(N[2]) ? 1 : 2);
        }

        u = (k + 1) % 3;
        v = (k + 2) % 3;

        const floating krec = 1.0 / N[k];
        nu = N[u] * krec;
        nv = N[v] * krec;
        nd = N.dot(A) * krec;

        const floating reci = 1.0 / (b[u] * c[v] - b[v] * c[u]);
        bnu =  b[u] * reci;
        bnv = -b[v] * reci;
        cnu =  c[v] * reci;
        cnv = -c[u] * reci;
    }

    void intersect (const Ray& ray, Intersection& intr) const override {
        const Point O = ray.origin();
        const Vector D = ray.dir();
        const int ku = (k + 1) % 3, kv = (k + 2) % 3;
        const floating lnd = 1.0 / (D[k] + nu * D[ku] + nv * D[kv]);
        const floating t = (nd - O[k] - nu * O[ku] - nv * O[kv]) * lnd;
        const floating hu = O[ku] + t * D[ku] - points[0][ku];
        const floating hv = O[kv] + t * D[kv] - points[0][kv];

        const auto u = hv * bnu + hu * bnv;
        const auto v = hu * cnu + hv * cnv;

        if (u < 0 || v < 0 || u + v > 1)
            return;

        intr.update (ray, this, t);
    }

#if 0
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
#endif

    Vector normal (const Point& pos) const override {
        return NormalBase::getNormal (pos);
    }

    floating getLeftExtreme(size_t axis) const override {
        return fmin(fmin(points[0][axis], points[1][axis]), points[2][axis]);
    }

    floating getRightExtreme(size_t axis) const override {
        return fmax(fmax(points[0][axis], points[1][axis]), points[2][axis]);
  }

    const Colour getColourAtIntersection (const Point& pos, const Texture* texture) const override {
        floating tu, tv;
        UVBase::getUV (pos, tu, tv);
        return texture->getTexel (tu, tv);
    }

private: /* Fields: */
    Point points[3];
    floating nu, nv, nd;
    floating bnu, bnv;
    floating cnu, cnv;
    uint8_t k;
};

template <typename NormalPolicy, typename UVPolicy>
inline Triangle<NormalPolicy, UVPolicy>* make_triangle (
        const Vertex<NormalPolicy, UVPolicy>& v1,
        const Vertex<NormalPolicy, UVPolicy>& v2,
        const Vertex<NormalPolicy, UVPolicy>& v3)
{
    return new Triangle<NormalPolicy, UVPolicy>{v1, v2, v3};
}

#endif

