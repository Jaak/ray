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
    const size_t x = clamp(round(u * width()), 0.0, width() - 1);
    const size_t y = clamp(round(v * height()), 0.0, height() - 1);

    return (*this)(x, y);
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
