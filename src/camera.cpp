#include "camera.h"
#include "scene.h"

#include <cmath>

Camera::Camera() { }

// TODO: init methods are bad
void Camera::init() {
  const Vector t = (m_at - m_eye).normalise();
  m_right = t.cross(m_up).normalise();
  m_up = m_right.cross(t).normalise();

  floating a = tan((m_fov * M_PI) / 360) * ((m_at - m_eye).length());
  m_right = a * m_right;
  m_up = a * m_up;
}

// TODO: this can be optimized quite a bit, almost all of the computation here
// can be cached inside camera object. This is ok, as we only have one of those
// anyways.
Ray Camera::spawnRay(floating hp, floating wp) const {
  const floating w = m_width;
  const floating h = m_height;
  const floating asp = w / h;
  const floating x = asp * ((((2 * wp) / (w - 1))) - 1.0);
  const floating y = -(((2 * hp) / (h - 1)) - 1.0);
  const Vector d = m_at + (m_right * x + m_up * y) - m_eye;
  const floating t = m_hither / ((m_eye - m_at).length());
  return { m_eye + t * d, normalised(d) };
}

std::ostream& operator<<(std::ostream& o, Camera const& cam) {
  o << "Camera: {\n";
  o << "Eye: " << cam.m_eye << '\n';
  o << "At: " << cam.m_at << '\n';
  o << "Right: " << cam.m_right << '\n';
  o << "Up: " << cam.m_up << '\n';
  o << "Hither: " << cam.m_hither << '\n';
  o << "FOV: " << cam.m_fov << '\n';
  o << '}';
  return o;
}
