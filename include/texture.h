#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"
#include "table.h"
#include <vector>

typedef int16_t texture_index_t;

class Texture : public table<Colour> {
public: /* Methods: */
  Texture (size_t w, size_t h)
    : table<Colour> (w, h)
  {}

  const Colour& getTexel(floating u, floating v) const {

  /*  if (interpolate) {
      const size_t x = clamp(round(u * width()), 0.0, width() - 1);
      const size_t y = clamp(round(v * height()), 0.0, height() - 1);

      return (*this)(x, y);
    }*/    

    floating x, y, x1, x2, y1, y2;
    Colour c11, c12, c21, c22;

    x = clamp(u * width(), 0.0, width() - 1);
    y = clamp(v * height(), 0.0, height() - 1);

    x1 = floor(x); x2 = ceil(x);
    y1 = floor(y); y2 = ceil(y);

    if (almost_zero(x2 - x1) || almost_zero(y2 - y1)) {
      return (*this)(round(x), round(y)); 
    }

    c11 = (*this)(x1, y1);
    c12 = (*this)(x1, y2);
    c21 = (*this)(x2, y1);
    c22 = (*this)(x2, y2);

    const Colour& result = (1.0 / (x2 - x1) * (y2 - y1)) *
                  (c11 * (x2 - x) * (y2 - y) +
                   c12 * (x - x1) * (y2 - y) +
                   c21 * (x2 - x) * (y - y1) +
                   c22 * (x - x1) * (y - y1)
                  );

    return result;
  }
    
};


class Textures {
    typedef std::vector<Texture> impl_t;

public: /* Methods: */
  Textures () {}

  texture_index_t registerTexture(Texture texture) {
    texture_index_t idx = m_textures.size();
    m_textures.push_back(texture);
    return idx;
  }

  const Texture* operator[](texture_index_t idx) const {
      return &m_textures[idx];
  }

private: /* Fields: */
  impl_t m_textures;
};

#endif
