#ifndef RAY_LIGHT_H
#define RAY_LIGHT_H

#include "geometry.h"

class Primitive;
class Ray;

enum LightType {
  AREA_LIGHT,
  POINT_LIGHT
};

class Light {
public: /* Methods: */
  explicit Light(Colour const& c, floating emission = 1.0)
    : m_colour(c)
    , m_emission (emission)
    , m_samplingPr (0.0)
  { }

  virtual ~Light() {}

  const Colour& colour(void) const { return m_colour; }

  floating emission () const { return m_emission; }

  // Sampling probability
  floating samplingPr () const { return m_samplingPr; }

  void setSamplingPr (floating pr) { m_samplingPr = 1.0; }

  /// Every light source must act as a part of the geometry.
  virtual const Primitive* prim () const = 0;

  inline Primitive* prim () {
    return const_cast<Primitive*>(static_cast<const Light*>(this)->prim());
  }

  /**
   * \brief Essentially sample a photon from the light source.
   * TODO: also return probability?
   */
  virtual Ray sample() const = 0;

  virtual floating lightPA () const = 0;

private: /* Fields: */
  Colour m_colour; ///< Colour of the light
  floating m_emission;
  floating m_samplingPr;
};

#endif
