#ifndef RAY_LIGHT_H
#define RAY_LIGHT_H

#include "primitive.h"

enum LightType {
  AREA_LIGHT,
  POINT_LIGHT
};

class Light {
public: /* Methods: */
  Light(Colour const& c) : m_colour(c) {}
  ~Light() {}

  Colour const& colour(void) const { return m_colour; }
  virtual LightType type(void) const = 0;
  virtual Ray emit() const = 0;

private: /* Fields: */
  Colour m_colour; ///< Colour of the light
};

#endif
