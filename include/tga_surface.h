#pragma once

#include <string>
#include "geometry.h"
#include "surface.h"

class TgaSurface : public Surface {
public: /* Methods: */

    TgaSurface(std::string fname)
        : data{nullptr}
        , m_fname{fname}
    { }

    ~TgaSurface();

    void init();

    void setPixel(int h, int w, const Colour& c) {
        data[h][w] = c.getPixel();
    }

private: /* Fields: */
    uint32_t** data;
    const std::string m_fname;
};
