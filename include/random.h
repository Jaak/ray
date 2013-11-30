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

inline floating rngNormal () {
    static auto dist = std::normal_distribution<floating>{0, 1};
    return dist (rng_engine ());
}

template <typename T>
inline T rngInt (const T n) {
    return std::uniform_int_distribution<T>{T {0}, n}(rng_engine ());
}

// TODO: this is quite inefficient, but definitely correct
inline Vector rngSphere() {
    return normalised (Vector { rngNormal (), rngNormal (), rngNormal ()});
}

inline Vector rngHemisphere () {
    const auto V = rngSphere ();
    return V.z < 0 ? - V : V;
}

inline Vector rngHemisphereVector (const Vector& dir) {
  const auto frame = Frame { dir };
  const auto localVec = rngHemisphere ();
  return frame.toWorld (localVec);
}

inline Vector sampleHemisphereVector (const Vector& dir, floating& cosT) {
    const auto frame = Frame { dir };
    const auto localVec = rngHemisphere ();
    cosT = localVec.z;
    return frame.toWorld (localVec);
}

#endif
