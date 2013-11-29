#ifndef RAY_PATHTRACER_H
#define RAY_PATHTRACER_H

#include "area_light.h"
#include "geometry.h"
#include "point_light.h"
#include "primitive.h"
#include "primitive_manager.h"
#include "random.h"
#include "ray.h"
#include "scene.h"
#include "vertex.h"
#include "table.h"

// just to force compilation
#include "frame.h"
#include "brdf.h"

#include <boost/range/adaptor/reversed.hpp>
#include <map>

constexpr floating emission = 800000.0;

// TODO: i think that our material representation is all wrong...

static inline EventType getEventType (const Material& m) {
    const auto total = m.kd () + m.ks () + m.t ();
    const auto r = total*rng ();
    if (r < m.kd ())           return DIFFUSE;
    if (r < m.kd () + m.ks ()) return REFLECT;
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
        return 0.0; // ??
    }
}

// TODO: proper brdf, this thing is a huge hack
Colour brdf (const Material& m, const Vector&, const Vector&) {
  if (reflectPr (m) > 0) return Colour { 0, 0, 0 };
  if (refractPr (m) > 0) return Colour { 0, 0, 0 };
    return m.colour () * (diffusePr (m) / M_PI + reflectPr (m) + refractPr (m));
}

  /*****************************
   * Bidirectional path tracer *
   *****************************/

class Pathtracer {
private: /* Methods: */

  Pathtracer& operator = (const Pathtracer&) = delete;
  Pathtracer(const Pathtracer&) = delete;

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
    const auto from2to = normalised(to.m_pos - from.m_pos);
    const auto to2from = - from2to;
    const auto testRay = Ray{ from.m_pos.nudgePoint(from2to), from2to};
    const auto intr = intersectWithPrims(testRay);
    const auto cosT0 = from.m_normal.dot(from2to);
    const auto cosT1 = to.m_normal.dot(to2from);

    if (cosT0 >= 0.0 && cosT1 >= 0.0 && fabs (intr.dist() - sqrt(sqLen)) < 2.0*ray_epsilon) {
      return (cosT0 * cosT1) / sqLen;
    } else {
      return 0.0;
    }
  }

  struct GeometricFactorCache : std::map<std::pair<const Vertex*, const Vertex*>, floating> {
  public: /* Methods: */
    
    explicit GeometricFactorCache (const Pathtracer& self)
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
    const Pathtracer& m_self;
  };

public: /* Methods: */

  explicit Pathtracer(Scene& s)
    : m_scene(s)
  { }

  Colour operator () (Ray ray) {
    return run (ray);
  }

private: /* Methods: */

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
    const auto N = prim->normal (point);
    const auto internal = V.dot(N) > 0.0;
    const auto into = ! internal;
    const auto N2 = internal ? -N : N;

    floating pr = std::max (objCol.r, std::max (objCol.g, objCol.b));

    // If some recursion depth is exeeded terminate with some probability
    if (depth > RAY_MAX_REC_DEPTH) {
        if (pr <= rng ()) {
            vertices.emplace_back (point, N, Colour { 0.0, 0.0, 0.0 }, prim, 0.0, DIFFUSE);
            return;
        }
    }
    else {
        pr = 1.0;
    }

    switch (getEventType (m)) {
    case LIGHT:
        vertices.emplace_back (point, N, objCol / M_PI, prim, pr / M_PI, LIGHT);
        break;
    case DIFFUSE: {
        const auto dir = rngHemisphereVector (N);
        const auto R = Ray { point.nudgePoint (dir), dir };
        vertices.emplace_back (point, N, objCol / M_PI, prim, pr / M_PI, DIFFUSE);
        trace (R, depth + 1, iior, vertices);
    }   break;
    case REFLECT: {
        const auto dir = reflect (V, N);
        const auto R = Ray { point.nudgePoint (dir), dir };
        vertices.emplace_back (point, N, objCol, prim, pr, REFLECT);
        trace (R, depth + 1, iior, vertices);
    }   break;
    case REFRACT: {
        // TODO: doesn't seem to be working yet...
        floating n1 = iior, n2 = m.ior();
        const auto n = into ? n1 / n2 : n2 / n1;
        const auto cosT1 = V.dot(N2);
        const auto cosT2 = 1.0 - n * n * (1.0 - cosT1 * cosT1);
        if (cosT2 < 0.0) {
            const auto reflDir = reflect (V, N);
            const auto R = Ray { point.nudgePoint (reflDir), reflDir };
            vertices.emplace_back(point, N2, objCol, prim, pr, REFLECT);
            trace (R, depth+1, iior, vertices);
            break;
        }

        const auto T = normalised (n * V - (into ? 1.0 : -1.0) * N * (n * cosT1 + sqrt (cosT2)));
        vertices.emplace_back(point, - N2, objCol, prim, pr, REFRACT);
        const auto R = Ray { point.nudgePoint (T), T };
        trace (R, depth+1, into ? n2 : n1, vertices);

//        const auto a = into ? n2 - n1 : n1 - n2;
//        const auto b = n1 + n2;
//        const auto R0 = (a * a) / (b * b);
//        const auto c = 1.0 - (into ? - cosT1 : T.dot(N));
//        const auto Re = R0 + (1.0 - R0) * pow (c, 5.0);
//        const auto Tr = 1.0 - Re;
//        const auto prob = 0.25 + 0.5 * Re;
//        if (rng () < prob) {
//            vertices.emplace_back(point, N, Re*objCol, prim, pr*prob, REFLECT);
//            const auto R = Ray { point.nudgePoint (reflDir), reflDir };
//            trace (R, depth+1, iior, vertices);
//            break;
//        }
//        else {
//            vertices.emplace_back(point, -N, Tr*objCol, prim, pr*(1.0 - prob), REFRACT);
//            const auto R = Ray { point.nudgePoint (T), T };
//            trace (R, depth+1, n2, vertices);
//            break;
//        }

    }   break;
    case EYE:
        assert (false && "No support for refraction in path tracer yet!");
        break;
    }
  }

  Colour run (const Ray& ray) {
      return radiance (traceEye (ray), traceLight ());
  }

  // TODO: should we add the initial point as a vertex?
  VertexList traceEye (const Ray& R) {
      VertexList vertices;
      vertices.reserve (RAY_MAX_REC_DEPTH);
      trace (R, 1, 1.0, vertices);
      return std::move (vertices);
  }

  // TODO: all lights emit same intensity light!
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

      std::vector<const Vertex*> xs;
      std::vector<floating> p;
      p.reserve (NL + NE + 2);
      xs.reserve (NL + NE + 2);

      // Equation 10.9
      table<double> w = { NL + 1, NE + 1, 0.0 };
      for (size_t s = 0; s <= NL; ++ s) {
          for (size_t t = 1; t <= NE; ++ t) {
              const size_t k = s + t - 1;
              if (k == 0) {
                  w (s, t) = 1.0;
                  continue;
              }

              xs.clear ();
              for (size_t i = 0; i < s; ++ i) xs.push_back (&lightVertices[i]);
              for (size_t i = 0; i < t; ++ i) xs.push_back (&eyeVertices[t - 1 - i]);

              p.resize (s + t + 1, 0.0);
              p[s] = 1.0;
              if (s == 0) {
                  p[1] = p[0] * PA_light / generationPr (xs[1], xs[0]) * gcache (xs[1], xs[0]);

                  for (size_t i = s + 1; i < k; ++ i) {
                      const auto denom = gcache (xs[i + 1], xs[i]) * generationPr (xs[i + 1], xs[i]);
                      p[i + 1] = p[i] * gcache (xs[i - 1], xs[i]) * generationPr (xs[i - 1], xs[i]) / denom;
                      if (denom <= 0.0)
                          p[i + 1] = 0.0;

                  }

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

private: /* Fields: */
  const Scene& m_scene;
};

#endif
