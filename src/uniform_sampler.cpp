#include "uniform_sampler.h"

#include "camera.h"
#include "geometry.h"
#include "random.h"
#include "renderer.h"

Colour UniformSampler::samplePixel(floating x, floating y) {
  auto c = Colour{ 0.0, 0.0, 0.0 };
  if (m_samples == 1) {
      c += renderer().render(camera().spawnRay(y + 0.5, x + 0.5));
  }
  else {
      for (size_t sample = 0; sample < m_samples; ++sample) {
        const auto dh = rng() - 0.5;
        const auto dw = rng() - 0.5;
        c += renderer().render(camera().spawnRay(y + dh, x + dw));
      }

      c = c / m_samples;
  }

  return c;
}
