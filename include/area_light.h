#ifndef RAY_AREA_LIGHT_H
#define RAY_AREA_LIGHT_H

#include "geometry.h"
#include "light.h"
#include "random.h"
#include "rectangle.h"

class AreaLight : public Light {
public: /* Methods: */

    AreaLight (const SceneSphere& sceneSphere, Colour intensity, Point p, Vector u, Vector v)
        : Light {sceneSphere, intensity, true, false}
        , m_point {p}
        , m_u {u}
        , m_v {v}
    {
        auto normal = u.cross (v);
        const auto len = normal.length (); // same as area
        normal = normal / len;
        m_frame = Frame::fromNormalised (normal);
        m_invArea = 1.0 / len;
    }

    IlluminateResult illuminate (Point pos) const override {
        const auto pointOnRectangle = m_point + rng () * m_u + rng () * m_v;
        auto direction = pointOnRectangle - pos;
        const auto distSqr = direction.sqrlength ();
        const auto distance = std::sqrt (distSqr);
        direction = direction / distance;
        const auto cosNormal = m_frame.normal () . dot(- direction);

        if (cosNormal < epsilon)
            return {};

        const auto directPdfW = m_invArea * distSqr / cosNormal;
        const auto emissionPdfW = m_invArea * cosNormal * RAY_INV_PI;
        return {intensity (), direction, distance, directPdfW, emissionPdfW, cosNormal};
    }

    EmitResult emit () const override {
        const auto sample = sampleCosHemisphere ();
        const auto localDir = sample.get ();
        const auto cosTheta = localDir.z;
        if (cosTheta < epsilon) // try again if angle is too steep
            return emit ();

        const auto pointOnRectangle = m_point + rng () * m_u + rng () * m_v;
        const auto direction = m_frame.toWorld (localDir);
        const auto emissionPdfW = m_invArea * sample.pdfW();
        const auto directPdfA = m_invArea;
        return {cosTheta * intensity (), pointOnRectangle,
                m_frame.normal (), direction,
                emissionPdfW, directPdfA, cosTheta
        };
    }

    RadianceResult radiance (Point, Vector dir) const override {
        const auto cosNormal = m_frame.normal ().dot (- dir);
        if (cosNormal <= 0.0)
            return {};

        return {intensity (), m_invArea * cosNormal * RAY_INV_PI, m_invArea};
    }

private: /* Fields: */
    const Point     m_point;
    const Vector    m_u;
    const Vector    m_v;
    Frame           m_frame;
    floating        m_invArea;
};

#endif
