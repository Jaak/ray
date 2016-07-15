#pragma once

#include "geometry.h"
#include "common.h"
#include "table.h"

#include <vector>

using texture_index_t = uint16_t;

class Texture : public table<Colour> {
public: /* Methods: */

    Texture(size_t w, size_t h)
        : table<Colour>(w, h)
    { }

    Colour getTexel(floating u, floating v) const {
        u = clamp(u * width() - 0.5, 0.0, width() - 2);
        v = clamp(v * height() - 0.5, 0.0, height() - 2);

        const auto x = floor(u);
        const auto y = floor(v);
        const auto u_ratio = u - x;
        const auto v_ratio = v - y;
        const auto u_opposite = 1.0 - u_ratio;
        const auto v_opposite = 1.0 - v_ratio;
        const auto c11 = (*this)(x + 0, y + 0);
        const auto c12 = (*this)(x + 1, y + 0);
        const auto c21 = (*this)(x + 0, y + 1);
        const auto c22 = (*this)(x + 1, y + 1);
        return (c11 * u_opposite + c12 * u_ratio) * v_opposite +
               (c21 * u_opposite + c22 * u_ratio) * v_ratio;
    }
};

class Textures {
    using impl_t = std::vector<Texture>;

public: /* Methods: */

    Textures() {}

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
