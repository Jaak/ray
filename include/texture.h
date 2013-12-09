#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"
#include <vector>

typedef int16_t texture_index_t;

class Texture {
public: /* Methods: */
  Texture(std::vector<std::vector<Colour>> texels, int w, int h) 
    : m_texels(texels)
    , m_width(w)
    , m_height(h)
    {}
  virtual ~Texture() {}
  std::vector<std::vector<Colour>> texels() const {return m_texels;}

  const Colour getTexel(floating u, floating v) const {
    int x = clamp(round(u * m_width), 0.0, m_width - 1);
    int y = clamp(round(v * m_height), 0.0, m_height - 1);

    return m_texels[y][x];
  }  

private: /* Fields: */
  std::vector<std::vector<Colour>> m_texels;
  int m_width;
  int m_height;
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
