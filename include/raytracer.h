#ifndef RAY_RAYTRACER_H
#define RAY_RAYTRACER_H

#include "area_light.h"
#include "geometry.h"
#include "point_light.h"
#include "primitive.h"
#include "primitive_manager.h"
#include "random.h"
#include "ray.h"
#include "scene.h"

#include <array>

struct Vertex {
  Vector     m_pos;
  Vector     m_normal;
  Colour     m_col;
  Primitive* m_prim;
  floating   m_pr;
};

// Possible to optimize this to std::array as we have bounded recursion depth
using VertexList = std::vector<Vertex>;

/// The raytracer function object class.
class Raytracer {
private: /* Methods: */

  Raytracer& operator = (const Raytracer&); // Not assignable.
  Raytracer(const Raytracer&); // Not copyable.

  Intersection intersectWithPrims(const Ray& ray) const {
    return m_scene.manager().intersectWithPrims(ray);
  }

  Ray shootRay(const Point& from, const Point& to) const {
    return shootRay(from, to - from);
  }

  Ray shootRay(const Point& from, Vector d) const {
    d.normalise();
    return { from + d*ray_epsilon, d };
  }

public:

  Raytracer(Scene& s)
    : m_scene(s)
    , m_col(0, 0, 0)
  { }

  const Colour& operator () (Ray ray) {
    run (ray);
    return m_col;
  }

private:

  /*****************************
   * Bidirectional path tracer *
   *****************************/

  void trace (const Ray& ray, size_t depth, floating iior, VertexList& vertices) {
    const auto intr = intersectWithPrims (ray);
    if (! intr.hasIntersections ())
      return;

    const auto prim = intr.getPrimitive ();
    const auto m = m_scene.materials ()[prim->material()];
    const auto objCol = m.colour();
    const auto point = intr.point();
    const auto V = ray.dir();
    const auto N = ray.normal(intr);
    const auto orientingN = N.dot(V) < 0.0 ? N : -1.0 * N;

    floating pr = std::max (objCol.r, std::max (objCol.g, objCol.b));
    if (depth > RAY_MAX_REC_DEPTH) {
    }
    else {
      pr = 1.0;
    }

  }

  /**********************
   * Simple ray tracer. *
   **********************/

  floating getShade(const PointLight& l, Point point, Vector& L) {
    const Point p = l.center();
    const Ray ray = shootRay(point, p);
    L = normalised(p - point);
    const Intersection intr = intersectWithPrims(ray);
    if (intr.hasIntersections() && intr.getPrimitive()->is_light()) {
      return 1.0;
    }

    return 0.0;
  }

  void run(const Ray& ray, size_t depth = 1, floating iior = 1.0,
           Colour acc = { 1, 1, 1 }) {
    if (depth > RAY_MAX_REC_DEPTH || acc.dot(acc) < RAY_MIN_COLOUR_TOLERANCE) {
      return;
    }

    const Intersection intr = intersectWithPrims(ray);

    if (!intr.hasIntersections()) {
      m_col += m_scene.background() * acc;
      return;
    }

    doLighting(ray, intr, depth, iior, acc);
  }

  void doLighting(const Ray& ray, const Intersection& intr, int depth,
                  floating iior, Colour acc) {

    const Primitive* prim = intr.getPrimitive ();
    const Material& m = m_scene.materials ()[prim->material()];

    if (prim->is_light()) {
      m_col += acc;
      return;
    }

    const Point point = intr.point();
    const Vector V = ray.dir();
    const Vector N = ray.normal(intr);

    for (const Light* l : m_scene.lights()) {
      Vector L;
      floating shade = 0;

      switch (l->type()) {
        case POINT_LIGHT:
          shade = getShade(*static_cast<const PointLight*>(l), point, L);
          break;
        default:
          assert (false && "Unsupported light type!");
          break;
      }

      // if not, then compute the lighting
      const floating dot_NL = L.dot(N);
      const Vector R = normalised(L - (2 * dot_NL) * N);
      const floating C = 1.0;
      const floating kd = m.kd() * fmax(0.0, dot_NL);
      const floating ks = m.ks() * pow(fmax(0.0, V.dot(R)), m.phong_pow());

      // diffuse
      m_col += C * (kd + ks) * acc * m.colour() * l->colour();
    }

    // reflection
    if (m.ks() > 0) {
      run(ray.reflect(intr), depth + 1, iior, m.ks() * acc);
    }

    // refraction
    if (m.t() > 0) {
      const Vector N2 = N; // * (intr.isExternal () ? -1 : 1);
      floating n1 = iior, n2 = m.ior();

      // TODO: right now we assume that we are always exiting to air
      // do we need to keep stack of refraction indices?
      if (intr.isInternal ()) {
        n1 = n2;
        n2 = 1.0;
      }

      const floating n = n1 / n2;
      const floating cosT1 = (-V).dot(N2);
      const floating cosT2 = 1.0 - n * n * (1.0 - cosT1 * cosT1);

      if (cosT2 > 0.0) {
        const floating cosT2sqrt = cosT1 > 0 ? - sqrt(cosT2) : sqrt(cosT2);
        const Vector T = n * V + (n * cosT1 + cosT2sqrt) * N2;
        Colour transp = { 1, 1, 1 }; // transparency of the object
        if (intr.isInternal()) { // Beer-Lambert law
          const Colour t = -intr.dist() * m.t() * m.colour();
          transp = expf (t);
        }

        run(shootRay(point, T), depth + 1, n2, acc * transp);
      }
    }
  }

private: /* Fields: */
  const Scene&  m_scene; ///< Reference to scene.
  Colour        m_col;
  VertexList    m_light_vertices;
  VertexList    m_eye_vertices;
};

#endif
