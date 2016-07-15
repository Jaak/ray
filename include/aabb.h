#pragma once

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
    floating split(Aabb& left, Aabb& right, size_t axis) const {
        left = right = *this;
        left.m_p2[axis] = (left.m_p1[axis] + left.m_p2[axis]) * 0.5;
        right.m_p1[axis] = left.m_p2[axis];
        return right.m_p1[axis];
    }

    /**
     * Splits box into two.
     * @see split
     */
    void split_at(Aabb& left, Aabb& right, size_t axis, floating pos) const {
        left = right = *this;
        left.m_p2[axis] = pos;
        right.m_p1[axis] = pos;
    }

    /// Surface area of the box.
    floating area() const {
        const Vector p = m_p2 - m_p1;
        return fabs(p[0] * p[1]) + fabs(p[0] * p[2]) + fabs(p[1] * p[2]);
    }

    /// Volume of the box.
    floating volume() const {
        const Vector p = m_p2 - m_p1;
        return fabs(p[0] * p[1] * p[2]);
    }

    Point m_p1; ///< Minimum point of the box
    Point m_p2; ///< Maximum point of the box.
};
