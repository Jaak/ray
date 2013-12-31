#ifndef RAY_FRAME_H
#define RAY_FRAME_H

#include "geometry.h"

/** 
 * Local frame of reference (only rotation and scaling, just 3x3 matrix).
 * Based on: https://github.com/SmallVCM/SmallVCM/blob/master/src/frame.hxx
 */
class Frame {
private:

    struct Normalised { const Vector dir; };

    explicit Frame (Normalised n)
        : m_z { n.dir }
        , m_y { pickOrthogonal (m_z) }
        , m_x { m_y.cross (m_z) }
    { }

public: /* Methods: */

  Frame ()
    : m_z {0, 0, 1}, m_y {0, 1, 0}, m_x {1, 0, 0}
  { }

  explicit Frame (Vector x, Vector y, Vector z)
    : m_z { z }, m_y { y }, m_x { x }
  { }

  explicit Frame (Vector dir)
    : m_z { normalised (dir) }
    , m_y { pickOrthogonal (m_z) }
    , m_x { m_y.cross (m_z) }
  { }

  Vector toWorld (Vector v) const {
    return v.x*m_x + v.y*m_y + v.z*m_z;
  }

  Vector toLocal (Vector v) const {
    return Vector { v.dot(m_x), v.dot(m_y), v.dot(m_z) };
  }

  const Vector& normal () const { return m_z; }
  const Vector& tangent () const { return m_y; }
  const Vector& binormal () const { return m_x; }

  // as the constructor normalises we can avoid redundant
  // normalisations using this
  static inline Frame fromNormalised (Vector dir) {
      return Frame { Normalised { dir } };
  }

private: /* Fields: */
  Vector m_z, m_y, m_x;
};

#endif /* RAY_FRAME_H */
