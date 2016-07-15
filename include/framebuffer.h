#pragma once

#include "geometry.h"
#include "table.h"

#include <boost/thread/mutex.hpp>

struct Aabb;
class Camera;

/**
 * All methods, apart from those starting with "unsafe" prefix are thread safe.
 */
class Framebuffer : table<Colour> {
public: /* Methods: */
    Framebuffer(size_t width, size_t height);
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    /**
     * Guarantees that the given pixel is in consistent state.  Does not flush
     * all framebuffer updates.
     */
    Colour getPixel(size_t x, size_t y) const;

    /**
     * Flush cached updates for the current thread.
     */
    void flushUpdates();

    /**
     * Clears the framebuffer to black, does not clear unflushed updates.
     */
    void clear();

    /**
     * This method caches the updates thread locally. Most of the time it
     * doesn't block and framebuffer lock is grabbed only when update buffer is
     * full.
     */
    void addColour(size_t x, size_t y, Colour col);

    // draw line on framebuffer. this is useful for debugging
    // (for instance to draw kd-tree, bounding boxes or traced rays)
    void unsafeDrawLine(floating fx0, floating fy0, floating fx1, floating fy1,
                        Colour col);
    void unsafeDrawAabb(const Camera& cam, const Aabb& box, Colour col);
    Colour unsafeGetPixel(size_t x, size_t y) const { return (*this)(x, y); }

    using table<Colour>::width;
    using table<Colour>::height;

private:
    using table<Colour>::operator();
    using table<Colour>::begin;
    using table<Colour>::end;

    inline void unsafePutColour(size_t x, size_t y, Colour col) {
        (*this)(x, y) = col;
    }
    inline void unsafeAddColour(size_t x, size_t y, Colour col) {
        (*this)(x, y) += col;
    }

private: /* Fields: */
    mutable boost::mutex m_mutex;
};
