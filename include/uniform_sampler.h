#ifndef RAY_UNIFORM_SAMPLER_H
#define RAY_UNIFORM_SAMPLER_H

#include "sampler.h"

class UniformSampler : public Sampler {
public: /* Methods: */

  UniformSampler (std::size_t samples)
    : m_samples (samples)
  { }

  Colour samplePixel (floating x, floating y);

private: /* Fields: */
  const std::size_t m_samples;
};

#endif
