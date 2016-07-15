#pragma once

#include "frame.h"
#include "geometry.h"
#include "light.h"
#include "random.h"

class DirectionalLight : public Light {
public: /* Methods: */

    DirectionalLight(const SceneSphere& sceneSphere, Colour intensity,
                     Vector direction)
        : Light{sceneSphere, intensity, false, true}
        , m_frame{direction}
    {}

    IlluminateResult illuminate(Point) const override {
        const auto emissionPdfW =
            concentricDiscPdfA() * sceneSphere().invRadiusSqr();
        return {intensity(),
                -m_frame.normal(),
                std::numeric_limits<floating>::max(),
                1.0,
                emissionPdfW,
                1.0};
    }

    EmitResult emit() const override {
        const auto sample = sampleConcentricDisc();
        const auto x = sample.get().x;
        const auto y = sample.get().y;
        const auto position = getPoint(x, y);
        const auto direction = m_frame.normal();
        const auto emissionPdfW = sample.pdfW() * sceneSphere().invRadiusSqr();
        return {intensity(),  position, direction, direction,
                emissionPdfW, 1.0,      1.0};
    }

    RadianceResult radiance(Point, Vector) const override {
        return {Colour{0, 0, 0}, 0.0, 0.0};
    }

private: /* Methods: */

    Point getPoint(floating x, floating y) const {
        const auto offset = sceneSphere().center() +
                            sceneSphere().radius() * (-m_frame.normal());
        const auto vecFromDiscCenter =
            m_frame.binormal() * x + m_frame.tangent() * y;
        return offset + sceneSphere().radius() * vecFromDiscCenter;
    }

private: /* Fields: */
    const Frame m_frame;
};
