#ifndef RAY_LIGHT_H
#define RAY_LIGHT_H

#include "geometry.h"

class SceneSphere {
public: /* Methods: */

    SceneSphere ()
        : m_center { 0, 0, 0 }
        , m_radius { 0 }
        , m_invRadiusSqr { 0 }
    { }

    void setCenter (Point p) { m_center = p; }
    void setRadius (floating r) {
        m_radius = r;
        m_invRadiusSqr = 1.0 / (r * r);
    }

    Point center () const { return m_center; }
    floating radius () const { return m_radius; }
    floating invRadiusSqr () const { return m_invRadiusSqr; }

private: /* Fields: */
    Point    m_center;
    floating m_radius;
    floating m_invRadiusSqr;
};

class Light {
public: /* Types: */

    /**
     * TODO: would it be possible to return less?
     * Seems like we have a lot of overlapp between the structures.
     */

    struct IlluminateResult {
        const Colour    radiance;
        const Vector    direction;
        const floating  distance;
        const floating  directPdfW;
        const floating  emissionPdfW;
        const floating  cosTheta;

        IlluminateResult ()
            : radiance {0, 0, 0}
            , direction {0, 0, 0}
            , distance {0}
            , directPdfW {0}
            , emissionPdfW {0}
            , cosTheta {0}
        { }

        IlluminateResult (Colour radiance, Vector direction,
                          floating distance, floating directPdfW,
                          floating emissionPdfW, floating cosTheta)
            : radiance {radiance}
            , direction {direction}
            , distance {distance}
            , directPdfW {directPdfW}
            , emissionPdfW {emissionPdfW}
            , cosTheta {cosTheta}
        { }
    };

    struct EmitResult {
        const Colour    energy;
        const Point     position;
        const Vector    normal;
        const Vector    direction;
        const floating  emissionPdfW;
        const floating  directPdfA;
        const floating  cosTheta;
    };

    struct RadianceResult {
        const Colour    radiance;
        const floating  emissionPdfW;
        const floating  directPdfA;

        RadianceResult ()
            : radiance {0, 0, 0}
            , emissionPdfW {0}
            , directPdfA {0}
        { }

        RadianceResult (Colour radiance, floating emissionPdfW, floating directPdfA)
            : radiance {radiance}
            , emissionPdfW {emissionPdfW}
            , directPdfA {directPdfA}
        { }
    };

public: /* Methods: */

  explicit Light (const SceneSphere& sceneSphere, Colour intensity, bool isFinite, bool isDelta)
    : m_sceneSphere { sceneSphere }
    , m_intensity { intensity }
    , m_finite { isFinite }
    , m_delta { isDelta }
  { }

  virtual ~Light() { }

  bool isFinite () const { return m_finite; }
  bool isDelta () const { return m_delta; }
  const SceneSphere& sceneSphere () const { return m_sceneSphere; }
  Colour intensity () const { return m_intensity; }
  floating samplingPr () const { return m_samplingPr; }
  void setSamplingPr (floating pr) { m_samplingPr = pr; }

  virtual IlluminateResult illuminate (Point pos) const = 0;
  virtual EmitResult emit () const = 0;
  virtual RadianceResult radiance (Point pos, Vector dir) const = 0;

private: /* Fields: */
  const SceneSphere&  m_sceneSphere;
  Colour              m_intensity;
  floating            m_samplingPr;
  bool                m_finite;
  bool                m_delta;
};

#endif
