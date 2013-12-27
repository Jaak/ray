#include "framebuffer.h"
#include "camera.h"
#include "aabb.h"

constexpr size_t edges[2*12] = {
    0, 1, 0, 2, 0, 3,
    1, 5, 1, 6,
    2, 4, 2, 6,
    3, 4, 3, 5,
    4, 7, 5, 7, 6, 7
};

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

    floating vs[2*8] = {};

    for (size_t i = 0; i < 8; ++ i)
        if (! cam.raster (sceneVertices[i], vs[2*i], vs[2*i + 1]))
            return;

    for (size_t i = 0; i < 12; ++ i) {
        const auto v0 = edges[2*i + 0]; const auto v1 = edges[2*i + 1];
        const auto x0 = vs[2*v0 + 0]; const auto y0 = vs[2*v0 + 1];
        const auto x1 = vs[2*v1 + 0]; const auto y1 = vs[2*v1 + 1];
        drawLine (x0, y0, x1, y1, col);
    }
}
