#ifndef RAY_PNG_SURFACE_H
#define RAY_PNG_SURFACE_H

#include <string>
#include "geometry.h"
#include "surface.h"

/**
 * A image.
 */
class PngSurface : public Surface {
public: /* Methods: */

  PngSurface(std::string fname)
      : data(nullptr), m_fname(fname) {}

 ~PngSurface();

	/**
	 * Initialise image.
	 * Allocates image data.
	 * setDimensions must be called before
	 * image can be initialised.
	 */
  void init();

  void setPixel(int h, int w, Colour const& c) {
    using namespace std;
    int& pix = data[h][w];
    pix = (int)(255 * fmin(1.0, fmax(c.r, 0.0))) << 16;
    pix |= (int)(255 * fmin(1.0, fmax(c.g, 0.0))) << 8;
    pix |= (int)(255 * fmin(1.0, fmax(c.b, 0.0)));
  }

private: /* Fields: */
	int** data; ///< All the pixels.
	std::string m_fname;
};

/**
 * @}
 */

#endif
