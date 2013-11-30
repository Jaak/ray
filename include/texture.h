#ifndef TEXTURE_H
#define TEXTURE_H


class Texture {
public: /* Methods: */
  Texture(std::vector<std::vector<Colour>> texels) : m_texels(texels) {}
  virtual ~Texture() {}
  std::vector<std::vector<Colour>> texels() {return m_texels;}
  

private: /* Fields: */
  std::vector<std::vector<Colour>> m_texels;

};

#endif
