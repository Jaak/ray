#ifndef RAY_CAMERA_H
#define RAY_CAMERA_H

#include "common.h"
#include "geometry.h"
#include "ray.h"

#include <iostream>

/// A camera.
class Camera {
public: /* Methods: */

  void setup (Point from, Point at, Vector up, floating fov, floating hither, size_t width, size_t height);

  Ray spawnRay(floating, floating) const;

  int height() const { return m_height; }
  int width() const { return m_width; }
  floating imagePlaneDistance () const { return m_imagePlaneDistance; }
  Vector forward () const { return m_forward; }

private: /* Fields: */
  size_t m_height;
  size_t m_width;
  Point m_eye;                   ///< Position of camera eye.
  Vector m_forward;              ///< (Unitary) direction the camera is looking at.
  floating m_imagePlaneDistance; ///< Distance from eye to image plane.
  Matrix m_toScreen;             ///< Model-view projection matrix.
  Matrix m_fromScreen;           ///< Matrix to project screen-coordinates to world.
};

#endif
