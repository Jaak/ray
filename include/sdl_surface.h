#ifndef RAY_SDL_SURFACE_H
#define RAY_SDL_SURFACE_H

#include "SDL/SDL.h"
#include "surface.h"
#include <boost/thread.hpp>

class OGLThread;

class SDLSurface : public Surface {
  friend class OGLThread;

public: /* Fields: */
  SDLSurface();
  ~SDLSurface();
  void init();
  void setPixel(int h, int w, Colour const&);

private: /* Methods: */
  Colour** m_buff;
  OGLThread* m_thread_data;
  boost::thread m_thread;
};

#endif
