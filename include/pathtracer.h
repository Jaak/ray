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
#include "table.h"
#include "renderer.h"
#include "brdf.h"

#include <map>

// TODO: i think that our material representation is all wrong...

inline floating diffusePr (const Material& m) {
    return m.kd () / (m.kd () + m.ks () + m.t ());
}

inline floating reflectPr (const Material& m) {
    return m.ks () / (m.kd () + m.ks () + m.t ());
}

inline floating refractPr (const Material& m) {
    return m.t () / (m.kd () + m.ks () + m.t ());
}

  /*****************************
   * Bidirectional path tracer *
   *****************************/

class Pathtracer : public Renderer {
public: /* Types: */

  enum EventType {
    DIFFUSE,
    REFLECT,
    REFRACT,
    LIGHT
  };

  struct Vertex {
    Point            m_pos;
    Vector           m_normal;
    Colour           m_col;
    union {
        const Primitive* m_prim;
        const Light* m_light;
    };

    floating         m_pr;
    EventType        m_event;

    Vertex (Point pos, Vector normal, Colour col, const Primitive* prim, floating pr, EventType event)
        : m_pos {pos}, m_normal {normal}, m_col {col}, m_prim {prim}, m_pr {pr}, m_event {event}
    { }

    Vertex (Point pos, Vector normal, Colour col, const Light* light, floating pr, EventType event)
        : m_pos {pos}, m_normal {normal}, m_col {col}, m_light {light}, m_pr {pr}, m_event {event}
    { }
  };

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

  using VertexList = std::vector<Vertex>;

private: /* Methods: */

  Pathtracer& operator = (const Pathtracer&) = delete;
  Pathtracer(const Pathtracer&) = delete;

  Intersection intersectWithPrims(const Ray& ray) const {
    return m_scene.manager().intersectWithPrims(ray);
  }

  static Ray shootRay(const Point& from, const Point& to) {
    return shootRay(from, to - from);
  }

  static Ray shootRay(const Point& from, Vector d) {
    d.normalise();
    return { from + d*ray_epsilon, d };
  }

  static inline EventType getEventType (const Material& m) {
      const auto total = m.kd () + m.ks () + m.t ();
      const auto r = total*rng ();
      if (r < m.kd ())           return DIFFUSE;
      if (r < m.kd () + m.ks ()) return REFLECT;
      return REFRACT;
  }

  static inline floating generationPr (const Vertex* from, const Vertex* to) {
      assert (from != nullptr && to != nullptr);

      switch (from->m_event) {
      case DIFFUSE:
          return 1.0 / M_PI;
      case REFLECT:
      case REFRACT:
          return 1.0;
      case LIGHT: {
          // const auto dir = normalised (to->m_pos - from->m_pos);
          // const auto cosT = from->m_normal.dot (dir);
          // return cosT <= 0.0 ? 0.0 : 1.0 / (2.0 * M_PI * cosT);
          const auto rad = from->m_light->radiance(from->m_pos, to->m_pos - from->m_pos);
          return rad.emissionPdfW;
          }
      }
  }

  // Function G defined in Definition 8.3 in Veach's thesis
  floating geometricFactor(const Vertex& from, const Vertex& to) const {
    const auto sqLen = (to.m_pos - from.m_pos).sqrlength();
    const auto from2to = normalised(to.m_pos - from.m_pos);
    const auto to2from = - from2to;
    const auto cosT0 = from.m_normal.dot(from2to);
    const auto cosT1 = to.m_normal.dot(to2from);

    if (cosT0 <= 0.0) return 0.0;
    if (cosT1 <= 0.0) return 0.0;

    const auto testRay = Ray{ from.m_pos.nudgePoint(from2to), from2to};
    const auto intr = intersectWithPrims(testRay);
    if (! intr.hasIntersections ())
        return 0.0;

    if (fabs (intr.dist() - sqrt(sqLen)) < 2.0*ray_epsilon) {
      return (cosT0 * cosT1) / sqLen;
    } else {
      return 0.0;
    }
  }

public: /* Methods: */

  explicit Pathtracer(const Scene& s)
    : Renderer { s }
  { }

  Colour render (Ray ray) {
    return run (ray);
  }

  Colour operator () (Ray ray) {
    return run (ray);
  }

private: /* Methods: */

  Colour getPrimColour (const Primitive* prim, const Point& point) const {
    const auto& m = m_scene.materials ()[prim->material()];
    if (prim->texture() >= 0) {
      const Texture* texture = m_scene.textures()[prim->texture()];
      return prim->getColourAtIntersection(point, texture);
    }

    return m.colour ();
  }


  // TODO: proper brdf, this thing is a huge hack
  inline Colour brdf (const Primitive* prim, const Point& p, const Vector&, const Vector&) {
      const auto& m = getMat (prim);
      if (reflectPr (m) > 0.0) return Colour { 0, 0, 0 };
      if (refractPr (m) > 0.0) return Colour { 0, 0, 0 };
      return getPrimColour (prim, p) * diffusePr (m) / M_PI;
  }

  void trace (Ray ray, VertexList& vertices) {
      size_t depth = 1;
      while (depth < 5) {
          const auto intr = intersectWithPrims (ray);
          if (! intr.hasIntersections ()) {
              return;
          }

          const auto prim = intr.getPrimitive ();
          const auto m = m_scene.materials ()[prim->material()];
          const auto point = intr.point();
          const auto objCol = getPrimColour (prim, point);
          const auto V = ray.dir();
          const auto N = prim->normal (point);
          const auto internal = V.dot(N) > 0.0;
          const auto N2 = internal ? -N : N;
          auto pr = clamp(luminance (objCol), 0, 1);

          if (depth > RAY_MAX_REC_DEPTH) {
              if (pr <= rng ()) {
                  vertices.emplace_back (point, N2, Colour { 0.0, 0.0, 0.0 }, prim, 0.0, DIFFUSE);
                  return;
              }
          }
          else {
              pr = 1.0;
          }

          if (prim->emissive ()) {
              vertices.emplace_back (point, N2, objCol / M_PI, prim, pr / M_PI, DIFFUSE);
              return;
          }

          switch (getEventType (m)) {
          case DIFFUSE: {
              const auto frame = Frame::fromNormalised (N2);
              const auto sample = sampleCosHemisphere ();
              const auto dir = frame.toWorld (sample.get ());
              vertices.emplace_back (point, N2, objCol / M_PI, prim, pr / M_PI, DIFFUSE);
              ray = shootRay (point, dir);
              break;
          }
          case REFLECT: {
              const auto dir = reflect (V, N2);
              vertices.emplace_back (point, N2, objCol, prim, pr, REFLECT);
              ray = shootRay (point, dir);
              break;
          }
          case REFRACT: {
              floating n1 = 1.0, n2 = m.ior();
              if (internal)
                  std::swap (n1, n2);
              const auto n = n1 / n2;
              const auto cosI = - V.dot(N2);
              const auto sinT2 = n * n * (1.0 - cosI * cosI);
              if (sinT2 > 1.0) { // TIR
                  const auto dir = reflect (V, N2);
                  vertices.emplace_back (point, N2, objCol, prim, pr, REFLECT);
                  ray = shootRay (point, dir);
                  break;
              }

              const auto cosT = std::sqrt (1.0 - sinT2);
              const auto T = normalised (n * V + N2 * (n * cosI - cosT));
              const auto Re = fresnel (cosI, n1, n2);
              const auto Tr = 1.0 - Re;
              if (rng () < Re) {
                  const auto reflDir = reflect (V, N2);
                  vertices.emplace_back(point, N2, Re*objCol, prim, pr*Re, REFLECT);
                  ray = shootRay (point, reflDir);
                  break;
              }
              else {
                  vertices.emplace_back(point, N2, Tr*objCol, prim, pr*Tr, REFRACT);
                  ray = shootRay (point, T);
                  break;
              }

              break;
          }
          }

          ++ depth;
      }
  }

  Colour run (const Ray& ray) {
      floating eyePA, lightPA;
      const auto evs = traceEye (ray, eyePA);
      const auto lvs = traceLight (lightPA);
      return radiance (eyePA, std::move (evs), lightPA, std::move (lvs));
  }

  // TODO: should we add the initial point as a vertex?
  VertexList traceEye (const Ray& R, floating& eyePA) {
      VertexList vertices;
      vertices.reserve (RAY_MAX_REC_DEPTH);
      eyePA = 1.0;
      trace (R, vertices);
      return std::move (vertices);
  }

  Light* pickLight () {
      floating acc = 0.0;
      for (const auto& l : m_scene.lights()) {
        acc += l->samplingPr();
        if (rng () <= acc) {
          return l.get();
        }
      }

      assert (acc >= 1.0);
      assert (false && "Unreachable code.");
      return nullptr;
  }

  VertexList traceLight (floating& lightPA) {
      assert (! m_scene.lights ().empty ());
      const auto light = pickLight ();
      const auto emission = light->emit ();
      lightPA = emission.directPdfA;
      VertexList vertices;
      vertices.reserve (RAY_MAX_REC_DEPTH);
      vertices.emplace_back (
        emission.position,
        emission.normal,
        emission.energy,
        light,
        clamp (1.0 / (2.0 * M_PI * emission.cosTheta), 0.0, 1.0),
        LIGHT);
      trace (shootRay(emission.position, emission.direction), vertices);
      return std::move (vertices);
  }

  const Material& getMat (const Primitive* prim) const {
      if (prim == nullptr)
          return m_scene.background ();
      return m_scene.materials ()[prim->material ()];
  }

  Colour radiance (floating eyePA, const VertexList& eyeVertices, floating lightPA, const VertexList& lightVertices) {
      const auto NE = eyeVertices.size ();
      const auto NL = lightVertices.size ();
      const auto alpha_L = computeAlphaL (lightPA, lightVertices);
      const auto alpha_E = computeAlphaE (eyePA, eyeVertices);

      // One below equation 10.8
      auto c = table<Colour> { NL + 1, NE + 1, { 0.0, 0.0, 0.0 } };
      auto gcache = GeometricFactorCache { *this };

      for (size_t t = 1; t <= NE; ++ t) {
          const auto prim = eyeVertices[t - 1].m_prim;
          if (prim && prim->emissive ()) {
            const auto l = prim->getLight ();
            c(0, t) = eyeVertices[t - 1].m_col*l->intensity();
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
                  brdf0 = brdf (lv.m_prim, lv.m_pos, lv.m_pos - lightVertices[s - 2].m_pos, ev.m_pos - lv.m_pos);
              }

              if (t == 1) {
                  brdf1 = brdf (ev.m_prim,  ev.m_pos, ev.m_pos - lv.m_pos, m_scene.camera ().eye () - ev.m_pos);
              }
              else {
                  brdf1 = brdf (ev.m_prim, ev.m_pos, ev.m_pos - lv.m_pos, eyeVertices[t - 2].m_pos - ev.m_pos);
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
                  const auto denom = generationPr (xs[1], xs[0]) * gcache (xs[1], xs[0]);
                  p[1] = p[0] * lightPA / denom;
                  if (denom == 0.0)
                      p[1] = 0.0;

                  for (size_t i = s + 1; i < k; ++ i) {
                      const auto denom = gcache (xs[i + 1], xs[i]) * generationPr (xs[i + 1], xs[i]);
                      p[i + 1] = p[i] * gcache (xs[i - 1], xs[i]) * generationPr (xs[i - 1], xs[i]) / denom;
                      if (denom <= 0.0)
                          p[i + 1] = 0.0;

                  }

                  p[k + 1] = p[k] * gcache (xs[k - 1], xs[k]) * generationPr (xs[k - 1], xs[k]) / eyePA;
              }
              else {

                  for (size_t i = s; i < k; ++ i) {
                      const auto denom = gcache(xs[i + 1], xs[i]) * generationPr(xs[i + 1], xs[i]);
                      p[i + 1] = p[i] * gcache(xs[i - 1], xs[i]) * generationPr(xs[i - 1], xs[i]) / denom;
                      if (denom <= 0.0) {
                          p[i + 1] = 0.0;
                      }
                  }

                  p[k + 1] = p[k] * gcache(xs[k - 1], xs[k]) * generationPr(xs[k - 1], xs[k]) / eyePA;

                  for (size_t i = s - 1; i > 0; -- i) {
                      const auto denom = gcache(xs[i - 1], xs[i]) * generationPr(xs[i - 1], xs[i]);
                      p[i] = p[i + 1] * gcache(xs[i + 1], xs[i]) * generationPr(xs[i + 1], xs[i]) / denom;
                      if (denom <= 0.0) {
                          p[i] = 0.0;
                      }
                  }

                  p[0] = p[1] * gcache(xs[1], xs[0]) * generationPr(xs[1], xs[0]) / lightPA;
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
              w (s, t) = fabs (sum) <= 0.0 ? 0.0 : 1.0 / sum;
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
  std::vector<Colour> computeAlphaL (floating lightPA, const VertexList& lightVertices) const {
      const auto NL = lightVertices.size ();
      auto alpha_L = std::vector<Colour>(NL + 1);

      alpha_L[0] = Colour { 1.0, 1.0, 1.0 };

      if (NL >= 1) {
        alpha_L[1] = (M_PI*lightVertices[0].m_col) / lightPA;
      }
      if (NL >= 2) {
          alpha_L[2] = ((1.0 / M_PI) / lightVertices[0].m_pr) * alpha_L[1];
      }
      for (size_t i = 3; i <= NL; ++ i) {
          alpha_L[i] = (lightVertices[i - 2].m_col / lightVertices[i - 2].m_pr) * alpha_L[i - 1];
      }
      return std::move (alpha_L);
  }

  // Equation 10.7
  std::vector<Colour> computeAlphaE (floating eyePA, const VertexList& eyeVertices) const {
      const auto NE = eyeVertices.size ();
      auto alpha_E = std::vector<Colour>(NE + 1);

      alpha_E[0] = Colour { 1.0, 1.0, 1.0 };

      if (NE >= 1)
          alpha_E[1] = Colour {1.0, 1.0, 1.0} / eyePA;

      for (size_t i = 2; i <= NE; ++ i)
          alpha_E[i] = (eyeVertices[i - 2].m_col / eyeVertices[i - 2].m_pr) * alpha_E[i - 1];

      return std::move (alpha_E);
  }
};

#endif
