#ifndef RAY_FRAMEBUFFER_H
#define RAY_FRAMEBUFFER_H

#include "geometry.h"
#include "table.h"

struct Aabb;
class Camera;

// TODO: this is actually taking up way too much memory. Each thread stores 3
// doubles per pixel. With 8 threads we need to store 8*1080*1080*3*8 bytes
// which is ~214 megabytes.  Possible solutions:
//  - use RGBE format to compress the colour (down to 36 MB)
//  - share a single framebuffer between threads (down to 27 MB)
//
// I leaning to the second solution. However, locking the framebuffer for every
// update is quite unsatisfactory solution. We can use thread local storage to
// cache updates and perform them in bulk. While updates are being flushed
// other threads can still perform theirs.
//
// The first solution has potential precision issues with converting between
// representations and performing arithmetic on RGBE pixels.
class Framebuffer : public table<Colour> {
public: /* Methods: */

    Framebuffer (size_t width, size_t height)
        : table<Colour> {width, height, Colour {0, 0, 0}}
    { }

    void clear () { fill (Colour(0, 0, 0)); }
    void addColour (floating x, floating y, Colour col) {
        (*this)((size_t) x, (size_t) y) += col;
    }

    // draw line on framebuffer. this is useful for debugging
    // (for instance to draw kd-tree, bounding boxes or traced rays)
    void drawLine (floating fx0, floating fy0, floating fx1, floating fy1, Colour col);
    void drawAabb (const Camera& cam, const Aabb& box, Colour col);

    using table<Colour>::operator ();
};

#endif
