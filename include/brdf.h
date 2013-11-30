#ifndef RAY_BRDF_H
#define RAY_BRDF_H

#include "material.h"
#include "frame.h"
#include "primitive.h"
#include "ray.h"
#include "geometry.h"
#include "vertex.h"


class BRDF {
public: /* Methods: */

    /**
     * @param ray Incoming ray.
     * @param normal Surface normal.
     * @param iior Incoming index of refraction (refraction of medium where the ray originated from).
     * @param mat Material of the surface.
     */
    BRDF (const Ray& ray, const Vector& normal, const floating iior, const Material& mat)
        : m_frame { normal }
        , m_localDirFix { m_frame.toLocal (- ray.dir ()) }
        , m_mat { mat }
        , m_ior { iior }
        , m_diffPr { 0.0 }
        , m_reflPr { 0.0 }
        , m_refrPr { 0.0 }
        , m_contPr { 0.0 }
        , m_fresPr { 0.0 }
    {
        const auto cosI = m_localDirFix.z;
        if (cosI < epsilon)
            return;

        // a portion of refracted light is actually reflected, this value
        // is given by the Fresnel term below:
        const auto fres = fresnel (cosI, iior, mat.ior ());
        const auto diff = mat.kd ();
        const auto refl = mat.ks () + fres * mat.t ();
        const auto refr = (1.0 - fres) * mat.t ();
        const auto total = diff + refl + refr;

        if (total < epsilon)
            return;

        const auto col = mat.colour ();
        m_diffPr = diff / total;
        m_reflPr = refl / total;
        m_refrPr = refr / total;
        m_contPr = clamp (std::max (col.r, std::max (col.g, col.b)), 0, 1);
    }

    Colour evaluate (Vector dirGen, floating& cosT) const {
        auto result = Colour { 0, 0, 0 };
        const auto localDirGen = m_frame.toLocal (dirGen);
        cosT = fmax (0.0, localDirGen.z);

        if (localDirGen.z < epsilon || m_localDirFix.z < epsilon)
            return result;

        if (m_diffPr > 0.0)
            result += m_mat.kd () * m_mat.colour () / M_PI;

        return result;
    }

    floating pdf (Vector dirGen) const {
        const auto localDirGen = m_frame.toLocal (dirGen);
        floating totalPdf = 0.0;

        if (m_localDirFix.z * localDirGen.z < 0.0)
            return totalPdf;

        if (m_diffPr > 0.0)
            totalPdf += m_diffPr * fmax (0.0, localDirGen.z) / M_PI;

        return totalPdf;
    }

    Vector sample (Vector dirGen, floating& pdf, floating& cosTGen, EventType& event) {
        const auto pr = rng ();
        event = REFRACT;
        if (pr < m_diffPr + m_reflPr) event = REFLECT;
        if (pr < m_diffPr) event = DIFFUSE;
    }


private: /* Fields: */
    Frame             m_frame;
    Vector            m_localDirFix;
    const Material&   m_mat;
    floating          m_ior;

    floating          m_diffPr; ///< Diffuse scattering probability.
    floating          m_reflPr; ///< Specular reflection probability.
    floating          m_refrPr; ///< Refraction probability.
    floating          m_contPr; ///< Continuation probability.
    floating          m_fresPr; ///< Fresnel coefficient -- probability to reflect instead of refract.
};

#endif /* RAY_BRDF_H */
