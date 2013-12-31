#ifndef RAY_SCENESPHERE_H
#define RAY_SCENESPHERE_H

#include "geometry.h"

class SceneSphere {
public: /* Methods: */

    SceneSphere ()
        : m_center { 0, 0, 0 }
        , m_radius { 0 }
        , m_invRadiusSqr { 0 }
    { }

    void setCenter (Point p) { m_center = p; }
    void setRadius (floating r) {
        m_radius = r;
        m_invRadiusSqr = 1.0 / (r * r);
    }

    Point center () const { return m_center; }
    floating radius () const { return m_radius; }
    floating invRadiusSqr () const { return m_invRadiusSqr; }

private: /* Fields: */
    Point    m_center;
    floating m_radius;
    floating m_invRadiusSqr;
};

#endif
