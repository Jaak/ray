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
  Light(Colour const& c) : m_colour(c) {}
  virtual ~Light() {}

  const Colour& colour(void) const { return m_colour; }

  /// Every light source must act as a part of the geometry.
  virtual const Primitive* prim () const = 0;

  /**
   * \brief Essentially sample a photon from the light source.
   * TODO: also return probability?
   */
  virtual Ray sample() const = 0;

private: /* Fields: */
  Colour m_colour; ///< Colour of the light
};

#endif
