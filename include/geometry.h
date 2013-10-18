#ifndef RAY_GEOMETRY_H
#define RAY_GEOMETRY_H

#include <iostream>
#include "common.h"

enum class Axes : uint8_t {
  X = 0, Y, Z, None
};

inline Axes nextAxis(Axes ax, unsigned d = 1) {
  return static_cast<Axes>((static_cast<unsigned>(ax) + d) % 3);
}

inline Axes& operator++(Axes& axis) {
  axis = (Axes)((int)(axis) + 1);
  return axis;
}

inline std::ostream& operator<<(std::ostream& os, Axes ax) {
  switch (ax) {
    case Axes::X: os << "X"; break;
    case Axes::Y: os << "Y"; break;
    case Axes::Z: os << "Z"; break;
    case Axes::None: os << "None"; break;
  }

  return os;
}

/************************
 * 4 dimensional vector *
 ************************/

class Vector {
public: /* Methods: */

  Vector() {}

  Vector(floating x, floating y, floating z, floating w = 0.0)
    : data{ x, y, z, w } {}

  floating& operator[](size_t i) { return data[i]; }
  floating operator[](size_t i) const { return data[i]; }
  floating& operator[](Axes i) { return data[static_cast<unsigned>(i)]; }
  floating operator[](Axes i) const { return data[static_cast<unsigned>(i)]; }

  Vector& operator+=(const Vector& v) {
    x += v.x; y += v.y; z += v.z; w += v.w;
    return *this;
  }

  Vector& operator-=(const Vector& v) {
    x -= v.x; y -= v.y; z -= v.z; w -= v.w;
    return *this;
  }

  Vector& operator*=(floating s) {
    x *= s; y *= s; z *= s; w *= s;
    return *this;
  }

  Vector& operator/=(floating s) { return (*this *= (floating(1.0) / s)); }

  floating dot(const Vector& v) const {
    return x * v.x + y * v.y + z * v.z + w * v.w;
  }

  floating sqrlength() const { return dot(*this); }

  floating length() const { return std::sqrt(sqrlength()); }

  Vector& normalise() {
    const floating len = sqrlength();
    return (*this /= std::sqrt(len));
  }

  Vector cross(const Vector& v) const {
    return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x };
  }

  Vector operator-() const {
    return { -x, -y, -z, -w };
  }

  friend std::ostream& operator<<(std::ostream& os, const Vector& v) {
    os << "Vector {" << v.x << "," << v.y << "," << v.z << "," << v.w << "}";
    return os;
  }

public: /* Fields: */

  union {
    floating data[4];
    struct { floating x, y, z, w; };
    struct { floating r, g, b, a; };
  };
};

inline Vector normalised(Vector u) { return u.normalise(); }

// whatever floats your boat
inline Vector normalized(Vector u) { return normalised(u); }

inline Vector operator+(const Vector& u, const Vector& v) {
  return { u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w };
}

inline Vector operator-(const Vector& u, const Vector& v) {
  return { u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w };
}

inline Vector operator*(floating s, const Vector& v) {
  return { s * v.x, s * v.y, s * v.z, s * v.w };
}

inline Vector operator*(const Vector& v, floating s) {
  return { v.x * s, v.y * s, v.z * s, v.w * s };
}

inline Vector operator/(const Vector& v, floating s) {
  return v * (floating(1.0) / s);
}

/***********************
 * 3 dimensional point *
 ***********************/

class Point : public Vector {
public: /* Methods: */

  Point() {}

  Point(floating x, floating y, floating z, floating w = 1.0)
      : Vector{ x, y, z, w } {}

  // Assume that the vector v is normalized
  Point nudgePoint(const Vector& v);

  floating dist(const Point& p) const { return (*this - p).length(); }

  floating sqDist(const Point& p) const { return (*this - p).sqrlength(); }

  friend std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << "Point {" << p.x << "," << p.y << "," << p.z << "," << p.w << "}";
    return os;
  }
};

inline Point operator+(const Point& p, const Vector& v) {
  return { p.x + v.x, p.y + v.y, p.z + v.z };
}

inline Point operator+(const Vector& v, const Point& p) { return p + v; }

inline Point Point::nudgePoint(const Vector& v) {
  return *this + ray_epsilon * v;
}

/****************************
 * representation of colour *
 ****************************/

class Colour : public Vector {
public: /* Methods: */

  Colour() {}

  Colour(floating r, floating g, floating b) : Vector{ r, g, b, 0 } {}

  Colour mult(const Colour& c) const {
    return { this->r * c.r, this->g * c.g, this->b * c.b };
  }

  floating diff(const Colour& c) const {
    return fabs(this->r - c.r) + fabs(this->g - c.g) + fabs(this->b - c.b);
  }

  uint32_t getPixel () const {
    using namespace std;
    uint32_t pix = 0;
    pix  = (uint32_t)(255 * fmin(1.0, fmax(r, 0.0))) << 16;
    pix |= (uint32_t)(255 * fmin(1.0, fmax(g, 0.0))) << 8;
    pix |= (uint32_t)(255 * fmin(1.0, fmax(b, 0.0)));
    return pix;
  }

  bool close(const Colour& c) const { return almost_zero(diff(c)); }

  friend std::ostream& operator<<(std::ostream& os, const Colour& p) {
    os << "Colour {" << p.r << "," << p.g << "," << p.b << "}";
    return os;
  }
};

inline Colour operator*(const Colour& c1, const Colour& c2) {
  return { c1.r * c2.r, c1.g * c2.g, c1.b * c2.b };
}

inline Colour operator/(const Colour& c1, floating s) {
  return { c1.r / s, c1.g / s, c1.b / s };
}

inline Colour operator+(const Colour& c1, const Colour& c2) {
  return { c1.r + c2.r, c1.g + c2.g, c1.b + c2.b };
}

inline Colour operator*(floating s, const Colour& c2) {
  return { s * c2.r, s * c2.g, s * c2.b };
}

#endif
