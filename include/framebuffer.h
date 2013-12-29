#ifndef RAY_FRAMEBUFFER_H
#define RAY_FRAMEBUFFER_H

#include "geometry.h"
#include "table.h"
#include <boost/thread/mutex.hpp>

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

/**
 * All methods, apart from those starting with "unsafe" prefix are thread safe.
 */
class Framebuffer : table<Colour> {
public: /* Methods: */

    Framebuffer (size_t width, size_t height)
        : table<Colour> {width, height, Colour {0, 0, 0}}
    { }

    Framebuffer (const Framebuffer&) = delete;
    Framebuffer& operator = (const Framebuffer&) = delete;

    /**
     * Guarantees that the given pixel is in consistent state.  Does not flush
     * all framebuffer updates.
     */
    Colour getPixel (size_t x, size_t y) const;

    /**
     * Flush cached updates for the current thread.
     */
    void flushUpdates ();

    /**
     * Clears the framebuffer to black, does not clear unflushed updates.
     */
    void clear();

    /**
     * This method caches the updates thread locally. Most of the time it
     * doesn't block and framebuffer lock is grabbed only when update buffer is
     * full.
     */
    void addColour (size_t x, size_t y, Colour col);

    // draw line on framebuffer. this is useful for debugging
    // (for instance to draw kd-tree, bounding boxes or traced rays)
    void unsafeDrawLine (floating fx0, floating fy0, floating fx1, floating fy1, Colour col);
    void unsafeDrawAabb (const Camera& cam, const Aabb& box, Colour col);
    Colour unsafeGetPixel (size_t x, size_t y) const { return (*this)(x, y); }

    using table<Colour>::width;
    using table<Colour>::height;

private:

    using table<Colour>::operator ();
    using table<Colour>::begin;
    using table<Colour>::end;

    inline void unsafePutColour (size_t x, size_t y, Colour col) { (*this)(x, y)  = col; }
    inline void unsafeAddColour (size_t x, size_t y, Colour col) { (*this)(x, y) += col; }

private: /* Fields: */
    mutable boost::mutex m_mutex;
};

#endif
