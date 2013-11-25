#ifndef RAY_PIXEL_H
#define RAY_PIXEL_H

#include "geometry.h"

class Pixel : Colour {
public: /* Methods: */

  explicit Pixel(const Colour& c = Colour(0, 0, 0))
    : Colour (c)
  { w = 0; }

  void reset() { r = g = b = w = 0.0; }
  const Colour& colour() const { return *this; }
  void setColour(const Colour& c) { r = c.r; g = c.g; b = c.b; }
  void activate() { w = 1.0; }
  bool active() const { return w > 0.5; }
};

#endif
