#ifndef RAY_SDL_SURFACE_H
#define RAY_SDL_SURFACE_H

#include "surface.h"
#include <memory>

class SDLThread;

class SDLSurface : public Surface {
public: /* Fields: */

  SDLSurface(const SDLSurface&) = delete;
  SDLSurface();
  ~SDLSurface();

  void init();
  void setPixel(int h, int w, const Colour&);

private: /* Methods: */
  std::shared_ptr<SDLThread> m_thread;
};

#endif
