#include "png_surface.h"

#include <cstdio>
#include <gd.h>
#include <iostream>

void PngSurface::init() {
  data = new int* [m_height];

  for (int i = 0; i < m_height; ++i) {
    data[i] = new int[m_width];
  }
}

PngSurface::~PngSurface() {
  gdImagePtr im;
  FILE* out;

  im = gdImageCreateTrueColor(width(), height());
  if (!(out = fopen(m_fname.c_str(), "wb"))) {
    std::cerr << "Unable to open/create file " << m_fname << std::endl;
    return;
  }

  for (int h = 0; h < m_height; ++h) {
    for (int w = 0; w < m_width; ++w) {
      gdImageSetPixel(im, w, h, data[h][w]);
    }
  }

  gdImagePng(im, out);
  fclose(out);
  gdImageDestroy(im);

  for (int i = 0; i < m_height; ++i) {
    delete[] data[i];
    data[i] = NULL;
  }

  delete[] data;
  data = NULL;
}
