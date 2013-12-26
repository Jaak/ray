#include "renderer.h"
#include "scene.h"
#include "framebuffer.h"
#include "camera.h"
#include "random.h"

void Renderer::render (Framebuffer& buf) {
    const auto& camera = m_scene.camera ();
    for (size_t x = 0; x < buf.width (); ++ x) {
        for (size_t y = 0; y < buf.height (); ++ y) {
            const auto dx = rng () - 0.5;
            const auto dy = rng () - 0.5;
            const auto ray = camera.spawnRay (x + dx, y + dy);
            buf.addColour (x, y, render (ray));
        }
    }
}
