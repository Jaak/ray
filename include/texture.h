#ifndef TEXTURE_H
#define TEXTURE_H

#include <vector>

typedef uint16_t texture_index_t;

class Texture {
public: /* Methods: */
  Texture(std::vector<std::vector<Colour>> texels) : m_texels(texels) {}
  virtual ~Texture() {}
  std::vector<std::vector<Colour>> texels() {return m_texels;}
  

private: /* Fields: */
  std::vector<std::vector<Colour>> m_texels;

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

    const Texture& operator[](texture_index_t idx) const {
        return m_textures[idx];
    }

private: /* Fields: */
    impl_t m_textures;
};

#endif
