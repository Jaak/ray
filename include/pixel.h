#ifndef RAY_PIXEL_H
#define RAY_PIXEL_H

#include "geometry.h"
#include "scene.h"

/**
 * A pixel.
 * Only used to perform anti-aliasing, caching of
 * traced rays specifically.
 * Pixel is active if it is already traced.
 *
 * Basically is kind of: Maybe Colour, but C++ is such
 * a crappy language that i have to write about hundred lines
 * of code and use dynamic memory to get that haskell kind
 * of behaviour of Maybe type.
 */
class Pixel {
public: /* Methods: */
  Pixel(const Colour& c = Colour(0, 0, 0)) : m_col(c), m_active(false) {}

  /**
   * Sets pixel colour to black and also
   * deactiavtes it.
   */
  void reset() {
    m_col = { 0, 0, 0 };
    m_active = false;
  }

  const Colour& colour() const { return m_col; }
  Colour& colour() { return m_col; }
  void setColour(const Colour& c) { m_col = c; }

  /// Activates pixel.
  void activate() { m_active = true; }

  /**
   * @retval true Pixel is active.
   * @retval false Pixel is inactive.
   */
  bool active() const { return m_active; }

private: /* Fields: */
  Colour m_col;  ///< Colour of the pixel.
  bool m_active; ///< Is the pixel active or not.
};

#endif
