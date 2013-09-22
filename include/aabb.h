#ifndef AABB_H
#define AABB_H

#include "geometry.h"

/**
 * Axis aligned bounding box.
 */
struct Aabb {

  /**
   * Splits the box to half.
   * @param left Left of the split box.
   * @param right Right of the split box.
   * @param axis Split axis.
   * @retval Coordinate of the split on @a axis.
   */
  floating split(Aabb& left, Aabb& right, Axes axis) const;

  /**
   * Splits box into two.
   * @see split
   */
  void split_at(Aabb&, Aabb&, Axes, floating) const;

  /// Surface area of the box.
  floating area() const {
    const Vector p = m_p2 - m_p1;
    return fabs(p[Axes::X] * p[Axes::Y]) +
           fabs(p[Axes::X] * p[Axes::Z]) +
           fabs(p[Axes::Y] * p[Axes::Z]);
  }

  /// Volume of the box.
  floating volume() const {
    const Vector p = m_p2 - m_p1;
    return fabs(p[Axes::X] * p[Axes::Y] * p[Axes::Z]);
  }

  friend std::ostream& operator<<(std::ostream& o, const Aabb& b) {
    o << "Aabb { " << b.m_p1 << "," << b.m_p2 << '}';
    return o;
  }

  Point m_p1; ///< Minimum point of the box
  Point m_p2; ///< Maximum point of the box.
};

#endif
