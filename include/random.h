#ifndef RANDOM_H
#define RANDOM_H

#include "common.h"
#include "frame.h"
#include "geometry.h"

#include <random>

typedef std::mt19937 Engine;

Engine& rng_engine ();

inline floating rng () {
  static auto dist = std::uniform_real_distribution<floating >{0, 1};
  return dist (rng_engine ());
}

// TODO: how slow is it to create a new distribution?
template <typename T>
inline T rngInt (const T n) {
    return std::uniform_int_distribution<T>{T {0}, n}(rng_engine ());
}

// TODO: make sure that this is uniform...
inline Vector rngHemisphere () {
  const auto T1 = 2 * M_PI * rng ();
  const auto z = sqrt (rng ());
  const auto xyproj = sqrt (1.0 - z*z);
  return Vector { xyproj*cos(T1), xyproj*sin(T1), z }; 
}

inline Vector rngSphere() {
  while (true) {
    const auto V = 2.0 * Vector { rng () - 0.5, rng () - 0.5, rng () - 0.5 };
    if (V.sqrlength () <= 1.0)
      return normalised (V);
  }
  // const auto f1 = 2.0 * rng() - 1.0f;
  // const auto f2 = 2.0 * M_PI * rng();
  // const auto r = sqrt(1.0 - f1 * f1);
  // return Vector{ r * cos(f2), r * sin(f2), f1 };
}

inline Vector rngHemisphereVector (const Vector& dir) {
  const auto frame = Frame { dir };
  const auto localVec = rngHemisphere ();
  return frame.toWorld (localVec);
//    const auto up = normalised (dir);
//    const auto u = normalised (up.cross (fabs(up.x) < 0.01 ? Vector {0, 1, 0} : Vector {1, 0, 0}));
//    const auto v = u.cross(up); // no need to normalise!
//    const auto H = rngHemisphere ();
//    const auto out = H.x*u + H.y*v + H.z*up;
//    return out;
}

inline Vector sampleHemisphereVector (const Vector& dir, floating& cosT) {
    const auto up = normalised (dir);
    const auto u = normalised (up.cross (fabs(up.x) < 0.01 ? Vector {0, 1, 0} : Vector {1, 0, 0}));
    const auto v = u.cross(up); // no need to normalise!
    const auto H = rngHemisphere ();
    const auto out = H.x*u + H.y*v + H.z*up;
    cosT = up.dot (out);
    return out;
}

#endif
