#include "framebuffer.h"
#include "camera.h"
#include "aabb.h"

void Framebuffer::drawAabb (const Camera& cam, const Aabb& box, Colour col) {
    const auto minToMax = box.m_p2 - box.m_p1;
    const auto u = Vector {minToMax[0], 0, 0 };
    const auto v = Vector {0, minToMax[1], 0 };
    const auto w = Vector {0, 0, minToMax[2] };
    const Point sceneVertices[8] = {
        box.m_p1, 
        box.m_p1 + u, box.m_p1 + v, box.m_p1 + w,
        box.m_p2 - u, box.m_p2 - v, box.m_p2 - w,
        box.m_p2
    };

    floating vs[8][2] = {};
    constexpr size_t edges[12][2] = {
          {0, 1}, {0, 2}, {0, 3}
        , {1, 5}, {1, 6}, {2, 4}, {2, 6}, {3, 4}, {3, 5}
        , {4, 7}, {5, 7}, {6, 7}
    };

    for (size_t i = 0; i < 8; ++ i)
        if (! cam.raster (sceneVertices[i], vs[i][0], vs[i][1]))
            return;

    for (size_t i = 0; i < 12; ++ i) {
        const auto v0 = edges[i][0]; const auto v1 = edges[i][1];
        const auto x0 = vs[v0][0]; const auto y0 = vs[v0][1];
        const auto x1 = vs[v1][0]; const auto y1 = vs[v1][1];
        drawLine (x0, y0, x1, y1, col);
    }
}

void Framebuffer::drawLine (floating fx0, floating fy0, floating fx1, floating fy1, Colour col) {
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
