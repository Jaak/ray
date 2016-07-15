#pragma once

#include "geometry.h"
#include "light.h"
#include "random.h"

class PointLight : public Light {
public: /* Methods: */
    PointLight(const SceneSphere& sceneSphere, Colour intensity, Point pos)
        : Light{sceneSphere, intensity, true, true}
        , m_position{pos}
    {}

    IlluminateResult illuminate(Point pos) const override {
        Vector         direction = m_position - pos;
        const floating distSqr = direction.sqrlength();
        const floating distance = std::sqrt(distSqr);
        direction = direction / distance;
        return {intensity(), direction,           distance,
                distSqr,     uniformSpherePdfW(), 1.0};
    }

    EmitResult emit() const override {
        const auto sample = sampleUniformSphere();
        const auto direction = sample.get();
        return {intensity(),   m_position, direction, direction,
                sample.pdfW(), 1.0,        1.0};
    }

    RadianceResult radiance(Point, Vector) const override {
        return {{0, 0, 0}, uniformSpherePdfW(), 1.0};
    }

private: /* Fields: */
    const Point m_position;
};

class SphereLight : public Light {
public: /* Methods: */
    SphereLight(const SceneSphere& sceneSphere, Colour intensity, Point center,
                floating r)
        : Light{sceneSphere, intensity, true, false}
        , m_center{center}
        , m_radius{r}
        , m_invArea{1.0 / (4.0 * M_PI * r * r)}
    {}

    // We are sampling points only from hemisphere facing the position.
    // Thus the area that we are sampling from is 2 times smaller.
    IlluminateResult illuminate(Point pos) const override {
        const auto normal = normalised(pos - m_center);
        const auto hemisphereVec = sampleUniformHemisphere().get();
        const auto localPosVec = Frame{normal}.toWorld(hemisphereVec);
        const auto pointOnHemisphere = m_center + m_radius * localPosVec;
        auto       direction = pointOnHemisphere - pos;
        const auto distSqr = direction.sqrlength();
        const auto distance = std::sqrt(distSqr);
        direction = direction / distance;
        const auto cosNormal = hemisphereVec.z;

        if (cosNormal < epsilon)
            return {};

        // 2 times smaller area
        const auto directPdfW = 2.0 * m_invArea * distSqr / cosNormal;
        const auto emissionPdfW = 2.0 * m_invArea * cosNormal * RAY_INV_PI;
        return {intensity(), direction,    distance,
                directPdfW,  emissionPdfW, cosNormal};
    }

    EmitResult emit() const override {
        const auto sample = sampleCosHemisphere();
        const auto localDir = sample.get();
        const auto cosTheta = localDir.z;
        if (cosTheta < epsilon)
            return emit();

        const auto localPosVec = sampleUniformSphere().get();
        const auto pointOnSphere = m_center + m_radius * localPosVec;
        const auto normal = normalised(pointOnSphere - m_center);
        const auto frame = Frame::fromNormalised(normal);
        const auto direction = frame.toWorld(localDir);
        const auto emissionPdfW = m_invArea * sample.pdfW();
        const auto directPdfA = m_invArea;
        return {cosTheta * intensity(), pointOnSphere, normal,  direction,
                emissionPdfW,           directPdfA,    cosTheta};
    }

    RadianceResult radiance(Point pos, Vector dir) const override {
        const auto normal = normalised(pos - m_center);
        const auto cosTheta = normal.dot(-dir);
        if (cosTheta <= 0.0)
            return {};

        return {intensity(), m_invArea * cosHemispherePdfW(normal, -dir),
                m_invArea};
    }

private: /* Fields: */
    const Point    m_center;
    const floating m_radius;
    const floating m_invArea; // this is the pdf of sampling on sphere uniformly
};
