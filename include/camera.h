#ifndef RAY_CAMERA_H
#define RAY_CAMERA_H

#include "common.h"
#include "geometry.h"
#include "ray.h"

#include <iostream>

class Scene;

/// A camera.
class Camera {
public:                 /* Methods: */
  Camera(const Scene&);

  /// Spawns a new ray through given screen coordinates.
  Ray spawnRay(floating, floating) const;

  void setHither(floating h) { m_hither = h; }
  void setEye(const Point& e) { m_eye = e; }
  void setAt(const Point& a) { m_at = a; }
  void setUp(const Vector& u) { m_up = normalised(u); }
  void setFOV(floating f) { m_fov = f; }

  /**
   * Initialises camera. Computes right vector and also
   * modifies up to be perpendicular to view direction vector.
   * Also modifies ups and rights length using fov.
   */
  void init();

  friend std::ostream& operator<<(std::ostream&, Camera const&);

private:             /* Fields: */
  const Scene & m_scene; ///< Reference to scene.
  Point m_eye;           ///< Position of camera eye.
  Point m_at;            ///< Point which camera looks at.
  Vector m_up;           ///< Up vector of camera.
  Vector m_right;        ///< Right vector of camera.
  floating m_hither;     ///< Clipping distance.
  floating m_fov;        ///< Field of view.
};

#endif
