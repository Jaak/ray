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

public: /* Methods: */

  explicit Raytracer(Scene& s)
    : m_scene(s)
  { }

  Colour operator () (Ray ray) {
    return run (ray);
  }

private: /* Methods: */


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

  Colour run(const Ray& ray, size_t depth = 1, floating iior = 1.0) {
    if (depth > RAY_MAX_REC_DEPTH) {
      return Colour {0, 0, 0};
    }

    const auto intr = intersectWithPrims(ray);

    if (!intr.hasIntersections()) {
      return m_scene.background().colour ();
    }
    else {
      return doLighting(ray, intr, depth, iior);
    }
  }

  Colour doLighting(const Ray& ray, const Intersection& intr, int depth, floating iior) {
    const auto prim = intr.getPrimitive ();
    const Material& m = m_scene.materials ()[prim->material()];

    if (prim->is_light()) {
      return Colour {1, 1, 1};
    }

    const auto point = intr.point();
    const auto V = ray.dir();
    const auto N = prim->normal (point);
    const auto internal = V.dot(N) > 0.0;
    const auto N2 = internal ? -N : N;

    auto col = Colour { 0, 0, 0 };

    for (const Light* l : m_scene.lights()) {
      Vector L;
      auto shade = getShade(l, point, L);
      if (almost_zero(shade)) {
        continue;
      }

      const auto R = reflect (L, N);
      const auto diff = m.kd () * clamp (L.dot(N), 0, 1);
      const auto spec = m.ks () * pow(clamp(V.dot(R), 0, 1), m.phong_pow());
      //col += (diff + spec) * m.colour() * l->colour();
      
      if (prim->texture() >= 0) {
        const Texture* texture = m_scene.textures()[prim->texture()];
        col += (diff + spec) * prim->getColourAtIntersection(point, texture) * l->colour();
      }
      else {
        col += (diff + spec) * m.colour() * l->colour();
      }

    }

    // reflection
    if (m.ks() > 0) {
      col += m.ks () * run(ray.reflect(intr), depth + 1, iior);
    }

    // refraction
    if (m.t() > 0) {
      floating n1 = iior, n2 = m.ior();

      const floating n = internal ? n2 / n1 : n1 / n2;
      const floating cosT1 = V.dot(N2);
      const floating cosT2 = 1.0 - n * n * (1.0 - cosT1 * cosT1);

      if (cosT2 > 0.0) {
        const auto T = normalised (n * V - (internal ? -1.0 : 1.0) * N * (n * cosT1 + sqrt (cosT2)));
        col += m.t () * run(shootRay(point, T), depth + 1, internal ? n2 : n1);
      }
    }

    return col;
  }

private: /* Fields: */
  const Scene& m_scene;
};

#endif
