
#include "geometry.h"
#include "SDL/SDL_opengl.h"
#include "sdl_surface.h"
#include <iostream>

class OGLThread {
public: /* Methods: */

  OGLThread(SDLSurface& surf)
    : m_screen(NULL), m_surf(surf), m_cont(true)
  {}

  void stop() { m_cont = false; }

  void init() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    m_screen =
        SDL_SetVideoMode(m_surf.m_width, m_surf.m_height, 32, SDL_OPENGL);
    SDL_WM_SetCaption("Tracing...", NULL);

    //glClearColor(0, 0, 0, 0);
    glViewport(0, 0, m_surf.m_width, m_surf.m_height);
    //glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_surf.m_width, m_surf.m_height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }

  void free() {
    SDL_Delay(5 * 1000);
    SDL_FreeSurface(m_screen);
    SDL_Quit();
  }

  void operator()() {
    init();

    while (m_cont) {
      glBegin(GL_POINTS);
      for (int h = 0; h < m_surf.m_height; ++h) {
        for (int w = 0; w < m_surf.m_width; ++w) {
          const Colour& c = m_surf.m_buff[h][w];
          glColor3f(c.r, c.g, c.b);
          glVertex2i(w, h);
        }
      }
      glEnd();
      glFlush();
      SDL_GL_SwapBuffers();
      boost::this_thread::sleep(boost::posix_time::milliseconds(60));
    }

    free();
  }

private: /* Fields: */
  SDL_Surface* m_screen;
  SDLSurface& m_surf;
  bool m_cont;
};

SDLSurface::SDLSurface() {}

SDLSurface::~SDLSurface() {
  m_thread_data->stop();
  m_thread.join();
  delete m_thread_data;
  for (int h = 0; h < m_height; ++h) {
    delete[] m_buff[h];
  }

  delete[] m_buff;
}

void SDLSurface::init(void) {
  m_buff = new Colour* [m_height];
  for (int h = 0; h < m_height; ++h) {
    m_buff[h] = new Colour[m_width];
  }

  m_thread_data = new OGLThread(*this);
  m_thread = boost::thread(boost::ref(*m_thread_data));
}

void SDLSurface::setPixel(int h, int w, Colour const& c) { m_buff[h][w] = c; }
