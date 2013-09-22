#ifndef RANDOM_H
#define RANDOM_H

#include "common.h"

#include <boost/random.hpp>

// TODO: fix this crap...

typedef boost::mt19937 Engine;
typedef boost::uniform_real<floating > Distribution;
typedef boost::variate_generator<Engine&, Distribution > RandomBase;

class Random : public RandomBase {
public: /* Methods: */
  Random() : RandomBase(m_gen, Distribution(0, 1)) {}

private: /* Fields: */
  Engine m_gen;
};

#endif
