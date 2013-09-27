
#include "geometry.h"
#include "sdl_surface.h"

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <SDL/SDL.h>
#include <iostream>

class SDLThread {
public: /* Methods: */

  SDLThread(const SDLThread&) = delete;

  SDLThread(int width, int height)
    : m_width(width)
    , m_height(height)
    , m_surf{ nullptr }
    , m_stopped{ false }
  {}

  void stop() { m_stopped = true; }

  void init() {
    SDL_Init(SDL_INIT_VIDEO);
    m_surf = SDL_SetVideoMode(m_width, m_height, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!m_surf) {
      return;
    }

    SDL_WM_SetCaption("Tracing...", NULL);
  }

  void setPixel(int h, int w, const Colour& c) {
    assert(m_surf);
    const auto r = (uint8_t)(255 * fmin(1.0, fmax(c.r, 0.0)));
    const auto g = (uint8_t)(255 * fmin(1.0, fmax(c.g, 0.0)));
    const auto b = (uint8_t)(255 * fmin(1.0, fmax(c.b, 0.0)));
    const auto bpp = m_surf->format->BytesPerPixel;
    const auto col = SDL_MapRGB(m_surf->format, r, g, b);
    const auto offset = h * m_surf->pitch + w * bpp;
    if (SDL_LockSurface(m_surf) == 0) {
      const auto pixels = (uint8_t*)m_surf->pixels;
      std::memcpy(pixels + offset, &col, bpp);
      SDL_UnlockSurface(m_surf);
    }
  }

  void run () {
    assert(m_surf);
    while (!m_stopped) {
      SDL_Flip(m_surf);
      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }

    SDL_FreeSurface(m_surf);
  }

private: /* Fields: */
  const int m_width;
  const int m_height;
  SDL_Surface* m_surf;
  bool m_stopped;
};

SDLSurface::SDLSurface() { }

SDLSurface::~SDLSurface() { }

void SDLSurface::init() {
  m_thread = std::make_shared<SDLThread>(m_width, m_height);
  m_thread->init();
  auto thread = boost::thread {
    [=](){ m_thread->run(); }
  };
}

void SDLSurface::setPixel(int h, int w, const Colour& c) {
  m_thread->setPixel(h, w, c);
}
