#ifndef RAY_RAYTRACER_H
#define RAY_RAYTRACER_H

#include "ray.h"
#include "primitive.h"
#include "primitive_manager.h"
#include "scene.h"
#include "geometry.h"
#include "random.h"
#include "area_light.h"
#include "point_light.h"

/// The raytracer function object class.
class Raytracer {
private: /* Methods: */

  Raytracer& operator = (const Raytracer&); // Not assignable.
  Raytracer(const Raytracer&); // Not copyable.

  Intersection intersectWithPrims(const Ray& ray) const {
    return m_scene.manager()->intersectWithPrims(ray);
  }

  Ray shootRay(const Point& from, const Point& to) const {
    return shootRay(from, to - from);
  }

  Ray shootRay(const Point& from, Vector d) const {
    d.normalise();
    return { from + d*ray_epsilon, d };
  }

public:

  Raytracer(Scene& s) : m_scene(s), m_col(0, 0, 0), gen() { }
  ~Raytracer() { }

  const Colour& operator () (Ray ray) {
    run (ray);
    return m_col;
  }

private:

  /**
   * TODO: the following is just a temporary hack for area lights.
   */
  floating getShade(const AreaLight& l, Point point, Vector& L) {
    floating out = 0.0;
    const int SAMPLES = 32;
    const floating is = 1.0 / (floating) SAMPLES;
    for (int i = 0; i < SAMPLES; ++i) {
      const Point p = l.emit(gen).origin();
      const Ray ray = shootRay(point, p);
      const Intersection intr = intersectWithPrims(ray);
      if (intr.hasIntersections() && intr.getPrimitive()->is_light()) {
        out += is;
      }
    }

    L = normalised(l.middle() - point);
    return out;
  }

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

  /**
   * @param iior Incoming index or refraction.
   * @param acc Accumulated colour multiplier.
   */
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

    const Point point = intr.point(); // point of intersection
    const Vector V = ray.dir(); // view direction vector
    const Vector N = ray.normal(intr);

    // for each light
    for (const Light* l : m_scene.lights()) {
      Vector L;
      floating shade = 0;

      switch (l->type()) {
        case POINT_LIGHT:
          shade = getShade(*static_cast<const PointLight*>(l), point, L);
          break;
        case AREA_LIGHT:
          shade = getShade(*static_cast<const AreaLight*>(l), point, L);
          break;
      }

      if (almost_zero(shade)) {
        continue;
      }

      // if not, then compute the lighting
      const floating dot_NL = L.dot(N);
      const Vector R = normalised(L - (2 * dot_NL) * N);
      const floating C = 1.0;
      const floating kd = m.kd() * fmax(0.0, dot_NL);
      const floating ks = m.ks() * pow(fmax(0.0, V.dot(R)), m.phong_pow());

      m_col += C * (kd + ks) * acc * m.colour() * l->colour();
    }

    // if primitives specular component is greater than 0 then reflect the ray
    if (m.ks() > 0) {
      run(ray.reflect(intr), depth + 1, iior, m.ks() * acc);
    }

    /**
     * Apply refraction.
     * Code is from Snell's law article in wikipedia
     * I'm not sure if this is correct.
     */
    if (m.t() > 0) {
      const Vector N2 = N * (intr.isExternal () ? -1 : 1);
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
        const Vector T = n * V + (n * cosT1 + sqrt(cosT2)) * N2;
        Colour transp = { 1, 1, 1 };
        if (intr.isInternal()) {
          const Colour t = -intr.dist() * 0.15 * m.colour();
          transp[Axes::X] = expf(t[Axes::X]);
          transp[Axes::Y] = expf(t[Axes::Y]);
          transp[Axes::Z] = expf(t[Axes::Z]);
        }

        run(shootRay(point, T), depth + 1, n2, m.t() * acc * transp);
      }
    }
  }

private: /* Fields: */
  const Scene&  m_scene; ///< Reference to scene.
  Colour        m_col;
  Random        gen;
};

#endif
