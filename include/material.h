#ifndef RAY_MATERIAL_H
#define RAY_MATERIAL_H

#include <cstdint>
#include "geometry.h"

typedef uint16_t material_index_t;

class Material {
public: /* Methods: */

  Material (Colour c = {1, 1, 1},
      floating kd = 1,
      floating pd = 1,
      floating ks = 0,
      floating ps = 0,
      floating t = 0,
      floating ior = 1,
      floating phong_pow = 1)
    : m_colour (c)
    , m_kd (kd)
    , m_pd (pd)
    , m_ks (ks)
    , m_ps (ps)
    , m_t (t)
    , m_ior (ior)
    , m_phong_pow (phong_pow)
  { }

  const Colour& colour () const { return m_colour; }
  floating kd () const { return m_kd; }
  floating pd () const { return m_pd; }
  floating ks () const { return m_ks; }
  floating ps () const { return m_ps; }
  floating t () const { return m_t; }
  floating ior () const { return m_ior; }
  floating phong_pow () const { return m_phong_pow; }

private: /* Fields: */
  const Colour    m_colour;     ///< Colour of the material.
  const floating  m_kd;         ///< Diffuse component.
  const floating  m_pd;         ///< Probability of diffuse reflection
  const floating  m_ks;         ///< Specular component.
  const floating  m_ps;         ///< Probability of specular reflection
  const floating  m_t;          ///< Transmittance.
  const floating  m_ior;        ///< Refraction.
  const floating  m_phong_pow;  ///< Phong constant.
};

#endif

