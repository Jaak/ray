#ifndef RAY_SPOTLIGHT_H
#define RAY_SPOTLIGHT_H

#include "light.h"
#include "random.h"

class Spotlight : public Light {
public: /* Methods: */

    Spotlight (const SceneSphere& sceneSphere, Colour intensity, const Point& p, const Vector& d, floating a)
        : Light {sceneSphere, intensity, true, true}
        , m_position {p}
        , m_frame {d}
        , m_cosAngle {clamp (cos (a), 0, 1)}
        , m_emissionPdfW {1.0 / (2.0 * M_PI * (1.0 - m_cosAngle)) }
    { }

    virtual IlluminateResult illuminate (Point pos) const override {
        Vector direction = m_position - pos;
        const floating distSqr = direction.sqrlength ();
        const floating distance = std::sqrt (distSqr);
        direction = direction / distance;

        if (m_frame.normal().dot(-direction) < m_cosAngle)
            return {};

        return {intensity (), direction, distance, distSqr, m_emissionPdfW, 1.0};
    }

    virtual EmitResult emit () const override {
        auto dir = Vector {0, 0, 0};
        while (true) {
            dir = sampleUniformHemisphere ().get ();
            if (dir.z >= m_cosAngle)
                break;
        }

        return {intensity (), m_position, m_frame.normal (), dir, m_emissionPdfW, 1.0, 1.0};
    }

    virtual RadianceResult radiance (Point, Vector) const override {
        return {{0, 0, 0}, m_emissionPdfW, 1.0};
    }

private: /* Fields: */
    const Point     m_position;
    const Frame     m_frame;
    const floating  m_cosAngle;
    const floating  m_emissionPdfW;
};

#endif
