#ifndef RAY_FRAMEBUFFER_H
#define RAY_FRAMEBUFFER_H

#include "geometry.h"
#include "table.h"

class Framebuffer : public table<Colour> {
public: /* Methods: */

    Framebuffer (size_t width, size_t height)
        : table<Colour> {width, height, Colour {0, 0, 0}}
    { }

    void clear () { fill (Colour(0, 0, 0)); }
    void addColour (floating x, floating y, Colour col) {
        (*this)((size_t) x, (size_t) y) += col;
    }

    using table<Colour>::operator ();
};

#endif
