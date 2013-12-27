#ifndef RAY_BRDF_H
#define RAY_BRDF_H

#include "material.h"
#include "frame.h"
#include "primitive.h"
#include "ray.h"
#include "geometry.h"

/**
 * TODO: can we compress this down some more?
 */
class BRDF {
public: /* Types: */

    enum Event {
        NONE,
        DIFFUSE,
        REFLECT,
        REFRACT
    };

    struct EvaluateResult {
        const Colour   colour;
        const floating cosTheta;
        const floating dirPdfW;
        const floating revPdfW;
    };

    struct PdfResult {
        const floating dirPdfW;
        const floating revPdfW;
    };

    struct SampleResult {
        const Colour   colour;
        const Vector   direction;
        const floating dirPdfW;
        const floating cosTheta;
        const Event    event;

        SampleResult ()
            : colour { 0, 0, 0}
            , direction { 0, 0, 0}
            , dirPdfW { 0 }
            , cosTheta { 0 }
            , event { NONE }
        { }

        SampleResult (Colour col, Vector dir, floating pdfW, floating cosTheta, Event event)
            : colour { col }
            , direction { dir }
            , dirPdfW { pdfW }
            , cosTheta { cosTheta }
            , event { event }
        { }

        bool valid () const {
            return event != NONE;
        }
    };

public: /* Methods: */

    /**
     * @param ray Incoming ray.
     * @param normal Surface normal.
     * @param iior Incoming index of refraction (refraction of medium where the ray originated from).
     * @param mat Material of the surface.
     */
    BRDF (const Ray& ray, const Vector& normal, const Material& mat)
        : m_frame { normal }
        , m_localDirFix { m_frame.toLocal (- ray.dir ()) }
        , m_mat (mat)
        , m_diffPr { 0.0 }
        , m_reflPr { 0.0 }
        , m_refrPr { 0.0 }
        , m_contPr { 0.0 }
        , m_fresPr { 0.0 }
        , m_valid { false }
        , m_delta { false }
    {
        const auto cosI = m_localDirFix.z;
        if (fabs (cosI) < epsilon)
            return;

        // a portion of refracted light is actually reflected, this value
        // is given by the Fresnel term below:
        const auto fres = mat.t () > 0 ? fresnel (cosI, 1.0, mat.ior ()) : 1.0;
        const auto diff = mat.kd ();
        const auto refl = fres * mat.ks ();
        const auto refr = (1.0 - fres) * mat.t ();
        const auto total = diff + refl + refr;

        if (total < epsilon) {
            m_valid = true;
            m_delta = true;
            return;
        }

        const auto col = mat.colour ();
        m_diffPr = diff / total;
        m_reflPr = refl / total;
        m_refrPr = refr / total;
        m_contPr = clamp (luminance(col), 0, 1);
        m_fresPr = fres;
        m_valid = true;
        m_delta = (m_diffPr == 0.0);
    }

    bool isValid () const { return m_valid; }
    bool isDelta () const { return m_delta; }
    floating continuationPr () const { return m_contPr; }
    floating cosThetaFix () const { return m_localDirFix.z; }
    Vector worldDirFix () const { return m_frame.toWorld (m_localDirFix); }

    EvaluateResult evaluate (Vector dirGen) const {
        Colour result = Colour {0, 0, 0};
        floating cosTheta = 0.0;
        floating dirPdfW = 0.0;
        floating revPdfW = 0.0;

        const auto localDirGen = m_frame.toLocal (dirGen);
        if (m_localDirFix.z * localDirGen.z >= 0.0) {
            cosTheta = fabs (localDirGen.z);
            evaluateDiffuse (localDirGen, result, dirPdfW, revPdfW);
            // evaluatePhong (localDirGen, result, dirPdfW, revPdfW);
        }

        return { result, cosTheta, dirPdfW, revPdfW };
    }

    PdfResult pdf (Vector dirGen) const {
        floating dirPdfW = 0.0;
        floating revPdfW = 0.0;

        const auto localDirGen = m_frame.toLocal (dirGen);
        if (localDirGen.z * m_localDirFix.z >= 0.0) {
            pdfDiffuse (localDirGen, dirPdfW, revPdfW);
            // pdfPhong (localDirGen, dirPdfW, revPdfW);
        }

        return { dirPdfW, revPdfW };
    }

    SampleResult sampleLight () const { return sample<true>(); }
    SampleResult sampleCamera () const { return sample<false>(); }

    template <bool LightTracing>
    SampleResult sample () const {
        auto col = Colour {0, 0, 0};
        auto dir = Vector {0, 0, 0};
        auto pdfW = floating {0};
        auto cosTheta = floating {0};
        auto event = NONE;

        {
            const auto r = rng ();
            if (r < m_diffPr) {
                event = DIFFUSE;
                sampleDiffuse (col, dir, pdfW);
            }
            else if (r < m_diffPr + m_reflPr) {
                event = REFLECT;
                sampleReflect (col, dir, pdfW);
            }
            else {
                event = REFRACT;
                sampleRefract<LightTracing>(col, dir, pdfW);
            }
        }

        cosTheta = fabs (dir.z);
        if (cosTheta < epsilon)
            return {};

        dir = m_frame.toWorld (dir);
        return {col, dir, pdfW, cosTheta, event};
    }

private: /* Methods: */

    // All private methods expect and return dirction in the local frame of reference

    void evaluateDiffuse (Vector localDirGen, Colour& result, floating& dirPdfW, floating& revPdfW) const {
        if (m_diffPr == 0.0)
            return;

        if (m_localDirFix.z < epsilon || localDirGen.z < epsilon)
            return;

        pdfDiffuse (localDirGen, dirPdfW, revPdfW);
        result  += m_mat.colour () / M_PI;
    }

    void pdfDiffuse (Vector localDirGen, floating& dirPdfW, floating& revPdfW) const {
        if (m_diffPr == 0.0)
            return;

        dirPdfW += m_diffPr * fmax (0.0, localDirGen.z / M_PI);
        revPdfW += m_diffPr * fmax (0.0, m_localDirFix.z / M_PI);
    }

    void sampleDiffuse (Colour& result, Vector& dir, floating& pdfW) const {
        if (m_localDirFix.z < epsilon)
            return;

        result += m_mat.colour () / M_PI;
        const auto sample = sampleCosHemisphere ();
        dir = sample.get ();
        pdfW += m_diffPr * sample.pdfW ();
    }

    void sampleReflect (Colour& result, Vector& dir, floating& pdfW) const {
        dir = Vector { - m_localDirFix.x, -m_localDirFix.y, m_localDirFix.z }; // reflect local over (0, 0, 1)
        pdfW += m_reflPr;
        result += m_mat.colour () * m_mat.ks()*m_fresPr / fabs (dir.z); /// XXX HACK to cancel the cos factor from outside
    }

    // TODO: always exiting to or entering from air
    template <bool LightTracing>
    void sampleRefract (Colour& result, Vector& dir, floating& pdfW) const {
        const bool internal = m_localDirFix.z < 0.0;
        const floating n = internal ? (m_mat.ior () / 1.0) : (1.0 / m_mat.ior ());
        const floating cosI = fabs (m_localDirFix.z);
        const floating sinT2 = n * n * (1.0 - cosI * cosI);
        if (sinT2 < 1.0) {
            const auto cosT = (internal ? 1.0 : -1.0) * std::sqrt (1.0 - sinT2);
            dir = Vector { - n*m_localDirFix.x, -n*m_localDirFix.y, cosT };
            pdfW += m_refrPr;
            const auto factor = LightTracing ? 1.0 : n*n;
            result += m_mat.colour()*m_mat.t()*(1.0 - m_fresPr)*factor / fabs (cosT); /// XXX HACK same as previous
        }
    }

private: /* Fields: */
    Frame             m_frame; ///< Local frame of reference (orthonormal basis).
    Vector            m_localDirFix; ///< The fixed local direction vector.
    const Material&   m_mat;

    floating          m_diffPr; ///< Diffuse scattering probability.
    floating          m_reflPr; ///< Specular reflection probability.
    floating          m_refrPr; ///< Refraction probability.
    floating          m_contPr; ///< Continuation probability.
    floating          m_fresPr; ///< Fresnel coefficient -- probability to reflect instead of refract.

    bool              m_valid : 1;
    bool              m_delta : 1;
};

#endif /* RAY_BRDF_H */
