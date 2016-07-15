#pragma once

#include "geometry.h"
#include "surface.h"

#include <string>

/**
 * A image.
 */
class PngSurface : public Surface {
public: /* Methods: */
    PngSurface(std::string fname)
        : data(nullptr)
        , m_fname(fname) {}

    ~PngSurface();

    /**
     * Initialise image.
     * Allocates image data.
     * setDimensions must be called before
     * image can be initialised.
     */
    void init();

    void setPixel(int h, int w, Colour const& c) { data[h][w] = c.getPixel(); }

private:              /* Fields: */
    int**       data; ///< All the pixels.
    std::string m_fname;
};
