#ifndef RAY_FRAMEBUFFER_H
#define RAY_FRAMEBUFFER_H

#include "geometry.h"
#include "table.h"

struct Aabb;
class Camera;

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
    void drawLine (floating fx0, floating fy0, floating fx1, floating fy1, Colour col) {
      fx0 = clamp (fx0, 0, width () - 1);
      fy0 = clamp (fy0, 0, height () - 1);
      fx1 = clamp (fx1, 0, width () - 1);
      fy1 = clamp (fy1, 0, height () - 1);
      int x0 = (int)fx0; const int x1 = (int)fx1;
      int y0 = (int)fy0; const int y1 = (int)fy1;
      const int dx = std::abs (x1 - x0);
      const int dy = std::abs (y1 - y0);
      const int sx = x0 < x1 ? 1 : -1;
      const int sy = y0 < y1 ? 1 : -1;
      int err = dx - dy;
      while (true) {
        (*this)(x0, y0) += col;
        if (x0 == x1 && y0 == y1)
          break;

        const int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (x0 == x1 && y0 == y1) { (*this)(x0, y0) += col; break; }
        if (e2 < dx) { err += dx; y0 += sy; }
      }
    }

    void drawAabb (const Camera& cam, const Aabb& box, Colour col);

    using table<Colour>::operator ();
};

#endif
