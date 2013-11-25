#ifndef RANDOM_H
#define RANDOM_H

#include "common.h"
#include "geometry.h"

#include <random>

typedef std::mt19937 Engine;

Engine& rng_engine ();

static inline floating rng () {
  static auto dist = std::uniform_real_distribution<floating >{0, 1};
  return dist (rng_engine ());
}

// TODO: make sure that this is uniform...
static inline Vector rngHemisphere () {
    const auto f1 = sqrt (rng ());
    const auto f2 = 2*M_PI*rng ();
    return Vector {f1*cos(f2), f1*sin(f2), sqrt(1.0 - f1*f1) };
}

static inline Vector rngHemisphereVector (const Vector& dir) {
    const auto up = normalised (dir);
    const auto u = normalised (up.cross (fabs(up.x) < 0.01 ? Vector {0, 1, 0} : Vector {1, 0, 0}));
    const auto v = u.cross(up); // no need to normalise!
    const auto H = rngHemisphere ();
    return H.x*u + H.y*v + H.z*up;

}

#endif
