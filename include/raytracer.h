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

#include <boost/range/adaptor/reversed.hpp>
#include <map>

enum EventType {
    DIFFUSE,
    REFLECT,
    REFRACT,
    LIGHT,
    EYE
};

constexpr floating emission = 200000.0;

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
  Colour           m_col; // TODO: use proper BRDF here
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

template <typename T>
class table : std::vector<T> {
public: /* Methods: */

    table (size_t width, size_t height, T def = T ())
        : std::vector<T>(width*height, def)
        , m_width { width }
        , m_height { height }
    { }

    T& operator () (size_t i, size_t j) {
        assert (i < m_width && j < m_height);
        return (*this)[i*m_height + j];
    }

    const T& operator () (size_t i, size_t j) const {
        assert (i < m_width && j < m_height);
        return (*this)[i*m_height + j];
    }

private: /* Fields: */
    const size_t m_width;
    const size_t m_height;
};

using VertexList = std::vector<Vertex>;

floating generationPr (const Vertex* from, const Vertex* to) {
    assert (from != nullptr && to != nullptr);
    switch (from->m_event) {
    case LIGHT:
        return 1.0 / M_PI; // ??
    case DIFFUSE: {
        const auto dir = normalised (to->m_pos - from->m_pos);
        const auto cosT = from->m_normal.dot (dir);
        return cosT <= 0.0 ? 0.0 : 1.0 / (2.0 * M_PI * cosT);
    }
    case REFLECT:
    case REFRACT:
        return 1.0;
    case EYE:
        return 1.0; // ??
    }
}

// TODO: proper brdf, this thing is a huge hack
Colour brdf (const Material& m, const Vector&, const Vector&) {
    return m.colour () * (diffusePr (m) / M_PI + reflectPr (m) + refractPr (m));
}

/**
 * The raytracer function object class. Determins colour of a single pixel.
 */
class Raytracer {
private: /* Methods: */

  Raytracer& operator = (const Raytracer&) = delete;
  Raytracer(const Raytracer&) = delete;

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


  // Function G defined in Definition 8.3 in Veach's thesis
  floating geometricFactor(const Vertex& from, const Vertex& to) const {
    const auto sqLen = (to.m_pos - from.m_pos).sqrlength();
    const auto to2from = normalised(to.m_pos - from.m_pos);
    const auto from2to = - to2from;
    const auto testRay = Ray{ from.m_pos.nudgePoint(to2from), to2from};
    const auto intr = intersectWithPrims(testRay);
    const auto cosT0 = from.m_normal.dot(to2from);
    const auto cosT1 = to.m_normal.dot(from2to);

    if (intr.hasIntersections () && almost_equal (intr.dist(), sqrt(sqLen))) {
      return fabs(cosT0 * cosT1) / sqLen;
    } else {
      return 0.0;
    }
  }

  struct GeometricFactorCache : std::map<std::pair<const Vertex*, const Vertex*>, floating> {
  public: /* Methods: */
    
    explicit GeometricFactorCache (const Raytracer& self)
      : m_self (self)
    { }

    floating operator () (const Vertex* from, const Vertex* to) {
      assert (from != nullptr && to != nullptr);
      const auto k = std::make_pair (from, to);
      auto it = find (k);
      if (it == end ()) {
        const auto factor = m_self.geometricFactor (*from, *to);
        it = insert (it, std::make_pair (k, factor));
      }

      return it->second;
    }

  private: /* Fields: */
    const Raytracer& m_self;
  };

public: /* Methods: */

  explicit Raytracer(Scene& s)
    : m_scene(s)
    , m_col(0, 0, 0)
  { }

  const Colour& operator () (Ray ray) {
      m_col = Colour { 0.0, 0.0, 0.0 };
      if (BPT_ENABLED) {
          runBPT (ray);
      }
      else {
          run (ray);
      }

      return m_col;
  }

private: /* Methods: */

  /*****************************
   * Bidirectional path tracer *
   *****************************/

  void trace (const Ray& ray, size_t depth, floating iior, VertexList& vertices) {
    const auto intr = intersectWithPrims (ray);

    if (! intr.hasIntersections ()) {
        return;
    }

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
            vertices.emplace_back (point, orientingN, Colour { 0.0, 0.0, 0.0 }, prim, 0.0, DIFFUSE);
            return;
        }
    }
    else {
        pr = 1.0;
    }

    switch (getEventType (m)) {
    case LIGHT:
        vertices.emplace_back (point, orientingN, objCol / M_PI, prim, pr / M_PI, LIGHT);
        return;
    case DIFFUSE: {
        const auto dir = rngHemisphereVector (orientingN);
        const auto R = Ray { point.nudgePoint (dir), dir };
        vertices.emplace_back (point, orientingN, objCol / M_PI, prim, pr / M_PI, DIFFUSE);
        trace (R, depth + 1, iior, vertices);
        return;
    }   break;
    case REFRACT:
    case REFLECT: {
        const auto dir = reflect (V, N);
        const auto R = Ray { point.nudgePoint (dir), dir };
        vertices.emplace_back (point, orientingN, objCol, prim, pr, REFLECT);
        trace (R, depth + 1, iior, vertices);
        return;
    }   break;
    case EYE:
        assert (false && "No support for refraction in path tracer yet!");
        abort ();
        break;
    }
  }

  void runBPT (const Ray& ray) {
      m_col = radiance (traceEye (ray), traceLight ());
  }

  // TODO: should we add the initial point as a vertex?
  VertexList traceEye (const Ray& R) {
      VertexList vertices;
      vertices.reserve (RAY_MAX_REC_DEPTH);
      trace (R, 1, 1.0, vertices);
      return std::move (vertices);
  }

  // TODO: all lights emit same intensity light!
  // TODO: we only support point lights for now!
  // TODO: should we add the initial point as a vertex?
  VertexList traceLight () {
      assert (! m_scene.lights ().empty ());
      const auto nLights = m_scene.lights ().size ();
      const floating lightPr = 1.0 / (floating) nLights;
      const auto light = m_scene.lights ()[nLights - 1];
      const auto R = light->sample ();
      const auto N = light->prim()->normal(R.origin ());


      VertexList vertices;
      vertices.reserve (RAY_MAX_REC_DEPTH);
      vertices.emplace_back (
        R.origin(), N, light->colour(), light->prim(),
        1.0 / (2.0 * M_PI * fmax (0.0, N.dot (R.dir ()))),
        LIGHT
      );
      trace (R, 1, lightPr, vertices);
      return std::move (vertices);
  }

  const Material& getMat (const Primitive* prim) const {
      if (prim == nullptr)
          return m_scene.background ();
      return m_scene.materials ()[prim->material ()];
  }

  Colour radiance (const VertexList& eyeVertices, const VertexList& lightVertices) {
      const auto NE = eyeVertices.size ();
      const auto NL = lightVertices.size ();

      const auto PA_light = floating { 1.0 };
      const auto PA_eye = floating { 1.0 };
      const auto alpha_L = computeAlphaL (lightVertices);
      const auto alpha_E = computeAlphaE (eyeVertices);

      // One below equation 10.8
      table<Colour> c = { NL + 1, NE + 1, { 0.0, 0.0, 0.0 } };
      auto gcache = GeometricFactorCache { *this };

      for (size_t t = 1; t <= NE; ++ t) {
          const auto prim = eyeVertices[t - 1].m_prim;
          if (prim && prim->is_light ()) {
              c(0, t) = eyeVertices[t - 1].m_col*emission;
          }
      }

      for (size_t s = 1; s <= NL; ++ s) {
          for (size_t t = 1; t <= NE; ++ t) {
              Colour brdf0, brdf1;
              const auto& lv = lightVertices[s - 1];
              const auto& ev = eyeVertices[t - 1];
              if (s == 1) {
                  brdf0 = Colour { 1.0, 1.0, 1.0 } / M_PI;
              }
              else {
                  brdf0 = brdf (getMat (lv.m_prim), lv.m_pos - lightVertices[s - 2].m_pos, ev.m_pos - lv.m_pos);
              }

              if (t == 1) {
                  brdf1 = brdf (getMat (ev.m_prim), ev.m_pos - lv.m_pos, m_scene.camera ().eye () - ev.m_pos);
              }
              else {
                  brdf1 = brdf (getMat (ev.m_prim), ev.m_pos - lv.m_pos, eyeVertices[t - 2].m_pos - ev.m_pos);
              }

              c(s, t) = gcache (&lv, &ev) * brdf0 * brdf1;
          }
      }

      // Equation 10.9
      table<double> w = { NL + 1, NE + 1, 0.0 };
      for (size_t s = 0; s <= NL; ++ s) {
          for (size_t t = 1; t <= NE; ++ t) {

              // sum_{i}(p_i/p_s)
              std::vector<floating> p (s + t + 1 , 0.0);
              const size_t k = s + t - 1;

              if (k == 0) {
                  w (s, t) = 1.0;
                  continue;
              }

              std::vector<const Vertex*> xs;
              xs.reserve (NL + NE);
              for (const auto& lv : lightVertices)
                  xs.push_back (&lv);
              for (const auto& ev : boost::adaptors::reverse (eyeVertices))
                  xs.push_back (&ev);

              p[s] = 1.0;
              if (s == 0) {
                  p[1] = p[0] * PA_light / generationPr (xs[1], xs[0]) * gcache (xs[1], xs[0]);
                  if (p[1] > 10e5) {
                      p[1] = 0.0;
                  }

                  for (size_t i = s + 1; i < k; ++ i) {
                      const auto denom = gcache (xs[i + 1], xs[i]) * generationPr (xs[i + 1], xs[i]);
                      p[i + 1] = p[i] * gcache (xs[i - 1], xs[i]) * generationPr (xs[i - 1], xs[i]) / denom;
                      if (denom <= 0.0)
                          p[i + 1] = 0.0;

                  }

                  if (k >= 1)
                      p[k + 1] = p[k] * gcache (xs[k - 1], xs[k]) * generationPr (xs[k - 1], xs[k]) / PA_eye;
              }
              else {

                  for (size_t i = s; i < k; ++ i) {
                      const auto denom = gcache(xs[i + 1], xs[i]) * generationPr(xs[i + 1], xs[i]);
                      p[i + 1] = p[i] * gcache(xs[i - 1], xs[i]) * generationPr(xs[i - 1], xs[i]) / denom;
                      if (denom <= 0.0) {
                          p[i + 1] = 0.0;
                      }
                  }

                  if (k >= 1)
                      p[k + 1] = p[k] * gcache(xs[k - 1], xs[k]) * generationPr(xs[k - 1], xs[k]) / PA_eye;

                  for (size_t i = s - 1; i > 0; -- i) {
                      const auto denom = gcache(xs[i - 1], xs[i]) * generationPr(xs[i - 1], xs[i]);
                      p[i] = p[i + 1] * gcache(xs[i + 1], xs[i]) * generationPr(xs[i + 1], xs[i]) / denom;
                      if (denom <= 0.0) {
                          p[i] = 0.0;
                      }
                  }

                  p[0] = p[1] * gcache(xs[1], xs[0]) * generationPr(xs[1], xs[0]) / PA_light;
              }

              for (size_t i = 0; i <= k; ++ i) {
                  switch (xs[i]->m_event) {
                  case REFLECT:
                  case REFRACT:
                      p[i] = p[i + 1] = 0.0;
                  default:
                      break;
                  }
              }

              floating sum = 0.0;
              for (auto x : p) sum += x*x;
              w (s, t) = fabs (sum) < epsilon ? 0.0 : 1.0 / sum;
          }
      }

      // Equation below 10.9
      auto result = Colour { 0.0, 0.0, 0.0 };
      for (size_t s = 0; s <= NL; ++ s) {
          for (size_t t = 1; t <= NE; ++ t) {
              result += w(s, t) * alpha_L[s] * c(s, t) * alpha_E[t];
          }
      }

      return result;
  }

  // Equation 10.6
  std::vector<Colour> computeAlphaL (const VertexList& lightVertices) const {
      const auto NL = lightVertices.size ();
      const auto PA_light = floating { 1.0 };
      auto alpha_L = std::vector<Colour>(NL + 1);

      alpha_L[0] = Colour { 1.0, 1.0, 1.0 };

      if (NL >= 1)
          alpha_L[1] = (M_PI*getMat (lightVertices[0].m_prim).colour ()*emission) / PA_light;
      if (NL >= 2)
          alpha_L[2] = (1.0 / M_PI) / lightVertices[0].m_pr * alpha_L[1];
      for (size_t i = 3; i <= NL; ++ i)
          alpha_L[i] = (lightVertices[i - 2].m_col / lightVertices[i - 2].m_pr) * alpha_L[i - 1];

      return std::move (alpha_L);
  }

  // Equation 10.7
  std::vector<Colour> computeAlphaE (const VertexList& eyeVertices) const {
      const auto NE = eyeVertices.size ();
      const auto PA_eye = floating { 1.0 };
      auto alpha_E = std::vector<Colour>(NE + 1);

      alpha_E[0] = Colour { 1.0, 1.0, 1.0 };

      if (NE >= 1)
          alpha_E[1] = Colour {1.0, 1.0, 1.0} / PA_eye;

      for (size_t i = 2; i <= NE; ++ i)
          alpha_E[i] = (eyeVertices[i - 2].m_col / eyeVertices[i - 2].m_pr) * alpha_E[i - 1];

      return std::move (alpha_E);
  }

  /**********************
   * Simple ray tracer. *
   **********************/

  // TODO: giant hack, we just sample a single point on the light source
  // and if the point isnt visible we are completely in shadow.
  floating getShade(const Light* l, Point point, Vector& L) {
    const Point p = l->sample ().origin ();
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
      m_col += m_scene.background().colour () * acc;
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
      floating shade = getShade(l, point, L);
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
};

#endif
