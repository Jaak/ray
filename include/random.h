#ifndef RANDOM_H
#define RANDOM_H

#include "common.h"

#include <random>

typedef std::mt19937 Engine;

Engine& rng_engine ();

static inline floating rng () {
  static auto dist = std::uniform_real_distribution<floating >{0, 1};
  return dist (rng_engine ());
}

#endif
