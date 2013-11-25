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

enum EventType {
    DIFFUSE,
    REFLECT,
    REFRACT
};

// TODO: i think that our material representation is all wrong...

static inline EventType getEventType (const Material& m) {
    const auto total = m.kd () + m.ks () + m.t ();
    const auto r = total*rng ();
    if (r < m.kd ())           return DIFFUSE;
    if (r < m.kd () + m.kd ()) return REFLECT;
    return REFRACT;
}

static inline floating diffusePr (const Material& m) {
    return m.kd () / (m.kd () + m.ks () + m.t ());
}

static inline floating reflectPr (const Material& m) {
    return m.ks () / (m.kd () + m.ks () + m.t ());
}

static inline floating refractPr (const Material& m) {
    return m.t () / (m.kd () + m.ks () + m.t ());
}

struct Vertex {
  Point            m_pos;
  Vector           m_normal;
  Colour           m_col;
  const Primitive* m_prim;
  floating         m_pr;
  EventType        m_event;

  Vertex (const Point& pos, const Vector& normal, const Colour& col, const Primitive* prim, floating pr, EventType event)
      : m_pos {pos}
      , m_normal {normal}
      , m_col {col}
      , m_prim {prim}
      , m_pr {pr}
      , m_event {event}
  { }
};

using VertexList = std::vector<Vertex>;

floating generationPr (const Vertex& from, const Vertex& to) {
    switch (from.m_event) {
    case DIFFUSE: {
        const auto dir = normalised (to.m_pos - from.m_pos);
        const auto cosT = from.m_normal.dot (dir);
        return cosT <= 0.0 ? 0.0 : 1.0 / (2.0 * M_PI * cosT);
    }
    case REFLECT:
    case REFRACT:
        return 1.0;
    }
}

/**
 * The raytracer function object class. Determins colour of a single pixel.
 */
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


  floating geometryTerm (const Vertex& from, const Vertex& to) {
      const auto sqLen = (to.m_pos - from.m_pos).sqrlength ();
      const auto from2to = normalised (from.m_pos - to.m_pos);
      const auto to2from = normalised (to.m_pos - from.m_pos);
      const auto testRay = Ray { from.m_pos.nudgePoint (from2to), from2to };
      const auto intr = intersectWithPrims (testRay);
      const auto c0 = from.m_normal.dot(from2to);
      const auto c1 = to.m_normal.dot(to2from);

      if (c0 >= 0.0 && c1 >= 0.0 && fabs (intr.dist () - sqrt(sqLen)) < epsilon) {
          return c0 * c1 / sqLen;
      }
      else {
           return 0.0;
      }
  }


public: /* Methods: */

  Raytracer(Scene& s)
    : m_scene(s)
    , m_col(0, 0, 0)
  { }

  const Colour& operator () (Ray ray) {
    run (ray);
    return m_col;
  }

private: /* Methods: */

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

    // If some recursion depth is exeeded terminate with some probability
    if (depth > RAY_MAX_REC_DEPTH) {
        if (pr <= rng ()) {
            // DIFFUSE? need some other kind of ray event?
            vertices.emplace_back (point, orientingN, m_scene.background (), prim, 0.0, DIFFUSE);
            return;
        }
    }
    else {
        pr = 1.0;
    }

    switch (getEventType (m)) {
    case DIFFUSE: {
        const auto dir = rngHemisphereVector (orientingN);
        const auto R = Ray { point.nudgePoint (dir), dir };
        vertices.emplace_back (point, orientingN, objCol / M_PI, prim, pr / M_PI, DIFFUSE);
        trace (R, depth + 1, iior, vertices);
    }   break;
    case REFLECT: {
        const auto dir = reflect (V, N);
        const auto R = Ray { point.nudgePoint (dir), dir };
        vertices.emplace_back (point, orientingN, objCol, prim, pr, REFLECT);
        trace (R, depth + 1, iior, vertices);
    }   break;
    case REFRACT:
        assert (false && "No support for refraction in path tracer yet!");
        break;
    }
  }

  VertexList traceEye (const Ray& R) {
      VertexList vertices;
      vertices.reserve (RAY_MAX_REC_DEPTH);
      trace (R, 1, 1.0, vertices);
      return std::move (vertices);
  }

  // TODO: all lights emit same intensity light!
  // TODO: we only support point lights for now!
  VertexList traceLight () {
      assert (! m_scene.lights ().empty ());
      const auto nLights = m_scene.lights ().size ();
      const floating lightPr = 1.0 / (floating) nLights;
      const auto light = m_scene.lights ()[nLights - 1];
      const auto R = light->emit ();

      VertexList vertices;
      vertices.reserve (RAY_MAX_REC_DEPTH);
      trace (R, 1, pr, vertices);
      return std::move (vertices);
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

      if (almost_zero(shade)) {
        continue;
      }

      // if not, then compute the lighting
      const Vector R = reflect (L, N);
      const floating C = 1.0;
      const floating kd = m.kd() * fmax(0.0, L.dot(N));
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
      const Vector N2 = N;
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
          const Colour t = -intr.dist() * 0.15 * m.colour();
          transp = expf (t);
        }

        run(shootRay(point, T), depth + 1, n2, m.t() * acc * transp);
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
