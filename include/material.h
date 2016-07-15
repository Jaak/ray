#pragma once

#include "geometry.h"

#include <cstdint>

using material_index_t = uint16_t;

class Material {
public: /* Methods: */
    Material(Colour c = {1, 1, 1}, floating kd = 1, floating ks = 0,
             floating t = 0, floating ior = 1, floating phong_pow = 1)
        : m_colour(c)
        , m_kd(kd)
        , m_ks(ks)
        , m_t(t)
        , m_ior(ior)
        , m_phong_pow(phong_pow)
    {}

    const Colour& colour() const { return m_colour; }

    floating kd() const { return m_kd; }
    floating ks() const { return m_ks; }
    floating t() const { return m_t; }
    floating ior() const { return m_ior; }
    floating phong_pow() const { return m_phong_pow; }

private: /* Fields: */
    const Colour   m_colour;    ///< Colour of the material.
    const floating m_kd;        ///< Diffuse component.
    const floating m_ks;        ///< Specular component.
    const floating m_t;         ///< Transmittance.
    const floating m_ior;       ///< Refraction.
    const floating m_phong_pow; ///< Phong constant.
};
