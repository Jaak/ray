#pragma once

#include "light.h"
#include "random.h"

class BackgroundLight : public Light {
public: /* Methods: */

    BackgroundLight(const SceneSphere& sceneSphere, Colour intensity)
        : Light{sceneSphere, intensity, false, false}
    {}

    IlluminateResult illuminate(Point) const override {
        const auto sample = sampleUniformSphere();
        const auto direction = sample.get();
        const auto directPdfW = sample.pdfW();
        const auto emissionPdfW =
            directPdfW * concentricDiscPdfA() * sceneSphere().invRadiusSqr();

        return {intensity(), direction,    std::numeric_limits<floating>::max(),
                directPdfW,  emissionPdfW, 1.0};
    }

    EmitResult emit() const override {
        // Sample direction:
        const auto dirSample = sampleUniformSphere();
        const auto direction = dirSample.get();

        // Sample position:
        const auto frame = Frame{direction};
        const auto discSample = sampleConcentricDisc();
        const auto offset =
            sceneSphere().center() + sceneSphere().radius() * (-direction);
        const auto x = discSample.get().x;
        const auto y = discSample.get().y;
        const auto vecFromDiscCenter =
            frame.binormal() * x + frame.tangent() * y;
        const auto position =
            offset + sceneSphere().radius() * vecFromDiscCenter;

        const auto emissionPdfW = dirSample.pdfW() * concentricDiscPdfA() *
                                  sceneSphere().invRadiusSqr();
        const auto directPdfA = dirSample.pdfW();

        return {intensity(),  position,   direction, direction,
                emissionPdfW, directPdfA, 1.0};
    }

    RadianceResult radiance(Point, Vector) const override {
        const auto directPdf = uniformSpherePdfW();
        const auto positionPdf =
            concentricDiscPdfA() * sceneSphere().invRadiusSqr();
        const auto emissionPdfW = directPdf * positionPdf;
        return {intensity(), emissionPdfW, directPdf};
    }
};
