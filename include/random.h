#ifndef RANDOM_H
#define RANDOM_H

#include "common.h"
#include "frame.h"
#include "geometry.h"

#include <random>

/**
 * Random number generation and sampling.
 */

typedef std::mt19937 Engine;

Engine& rng_engine ();

inline floating rng () {
  static auto dist = std::uniform_real_distribution<floating >{0, 1};
  return dist (rng_engine ());
}

template <typename T>
inline T rngInt (const T n) {
    return std::uniform_int_distribution<T>{T {0}, n}(rng_engine ());
}

/**
 * Samples have the probability density function (with respect to solid angle)
 * attached.
 */

template <typename T>
class Sample {
public: /* Methods: */

    explicit Sample (T value, floating pdfW)
        : m_value (value), m_pdfW (pdfW)
    { }

    inline T get () const { return m_value; }
    inline floating pdfW () const { return m_pdfW; }

private: /* Fields: */
    const T         m_value;
    const floating  m_pdfW;
};

template <typename T>
inline Sample<T> make_sample (T value, floating pdfW) {
    return Sample<T>{std::move(value), pdfW};
}

/**
 * Uniformly distributed unit vector.
 */

inline floating uniformSpherePdfW () {
    return 1.0 / (4.0 * M_PI);
}

inline Sample<Vector> sampleUniformSphere () {
    const auto r1 = rng ();
    const auto r2 = rng ();
    const auto T1 = 2.0 * M_PI * r1;
    const auto T2 = 2.0 * std::sqrt (r2*(1.0 - r2));
    const auto vec = Vector { cos(T1)*T2, sin(T1)*T2, 1.0 - 2.0*r2 };
    return make_sample (vec, uniformSpherePdfW ());
}

inline floating uniformHemispherePdfW () {
    return 1.0 / (2.0 * M_PI);
}

inline Sample<Vector> sampleUniformHemisphere () {
    auto vec = sampleUniformSphere ().get ();
    if (vec.z < 0.0) vec.z = -vec.z;
    return make_sample (vec, uniformHemispherePdfW ());
}

/**
 * cos-weight unit vector in direction (0, 0, 1).
 * Meaning that the z component is always non-negative.
 */

inline Sample<Vector> sampleCosHemisphere () {
    const auto r1 = rng ();
    const auto r2 = rng ();
    const auto T1 = 2.0 * M_PI * r1;
    const auto T2 = std::sqrt (1.0 - r2);
    const auto vec = Vector { cos(T1)*T2, sin(T1)*T2, std::sqrt(r2) };
    return make_sample (vec, vec.z * RAY_INV_PI);
}

inline floating localCosHemispherePdfW (Vector v) { return v.z * RAY_INV_PI; }

inline floating cosHemispherePdfW (Vector normal, Vector dir) {
    return clamp (normal.dot(dir), 0, 1) * RAY_INV_PI;
}

#endif
