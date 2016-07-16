#include "tga_surface.h"

#include <memory>
#include <iostream>

namespace {

class PixelDtor {
public: /* Methods: */
    PixelDtor(uint32_t** data, size_t height)
        : data(data)
        , height(height) {}

    ~PixelDtor() {
        for (size_t i = 0; i < height; ++i)
            delete[] data[i];
        delete[] data;
    }

private: /* Fields: */
    uint32_t** const data;
    size_t const     height;
};

} //  namespace anonymous

void TgaSurface::init() {
    data = new uint32_t*[m_height];

    for (int i = 0; i < m_height; ++i) {
        data[i] = new uint32_t[m_width];
    }
}

TgaSurface::~TgaSurface() {
    const PixelDtor pixel_dtor{data, (size_t)m_height};
    FILE*           out = fopen(m_fname.c_str(), "wb");
    std::unique_ptr<FILE, int (*)(FILE*)> file_closer = {out, &fclose};

    if (out == nullptr) {
        std::cerr << "Unable to open/create file " << m_fname << std::endl;
        return;
    }

    // Write header
    // http://www.paulbourke.net/dataformats/tga/
    fputc(0, out);
    fputc(0, out);
    fputc(2, out);
    fputc(0, out);
    fputc(0, out);
    fputc(0, out);
    fputc(0, out);
    fputc(0, out);
    fputc(0, out);
    fputc(0, out);
    fputc(0, out);
    fputc(0, out);
    fputc((m_width >> 0) & 0xff, out);
    fputc((m_width >> 8) & 0xff, out);
    fputc((m_height >> 0) & 0xff, out);
    fputc((m_height >> 8) & 0xff, out);
    fputc(24, out);
    fputc(0, out);

    for (int h = m_height; h-- > 0;) {
        for (int w = 0; w < m_width; ++w) {
            uint32_t pix = data[h][w];
            fputc((pix >> 0) & 0xff, out);
            fputc((pix >> 8) & 0xff, out);
            fputc((pix >> 16) & 0xff, out);
        }
    }
}
