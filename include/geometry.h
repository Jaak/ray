#ifndef RAY_GEOMETRY_H
#define RAY_GEOMETRY_H

#include "common.h"

#include <cstdint>
#include <iosfwd>

/*************
 * 2d vector *
 **************/

class Vector2 {
public: /* Methods: */

    Vector2 () { }

    Vector2 (floating x, floating y)
        : x {x}, y {y}
    { }

    floating& operator[](size_t i) { return data[i]; }
    floating operator[](size_t i) const { return data[i]; }

public: /* Fields: */
  union {
    floating data[2];
    struct { floating x, y; };
  };
};

/************************
 * 4 dimensional vector *
 ************************/

class Vector {
public: /* Methods: */

  Vector() {}

  Vector(floating x, floating y, floating z, floating w = 0.0)
    : data{ x, y, z, w }
  {}

  floating& operator[](size_t i) { return data[i]; }
  floating operator[](size_t i) const { return data[i]; }

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

  friend std::ostream& operator<<(std::ostream& os, const Vector& v);

public: /* Fields: */

  union {
    floating data[4];
    struct { floating x, y, z, w; };
    struct { floating r, g, b, a; };
  };
};

inline Vector normalised(Vector u) { return u.normalise(); }

inline Vector normalised (floating x, floating y, floating z) {
    return normalised (Vector {x, y, z});
}

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

inline Vector pickOrthogonal (const Vector& dir) {
  const Vector X { 1, 0, 0 };
  const Vector Y { 0, 1, 0 };
  return normalised (dir.cross(fabs(dir.x) < 0.01 ? X : Y));
}

inline Vector reflect (const Vector& I, const Vector& N) {
  return I - 2.0 * N.dot(I) * N;
}

// angle between two vectors in radians
inline floating getAngle (const Vector& v1, const Vector& v2) {
  return acos(v1.dot(v2) / (v1.length() * v2.length()));
}

// angle between two normalised vectors in radians
inline floating getAngleN (const Vector& v1, const Vector v2) {
  return acos(v1.dot(v2));
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
    Point nudgePoint(const Vector& v) const;

    floating dist(const Point& p) const { return (*this - p).length(); }

    floating sqDist(const Point& p) const { return (*this - p).sqrlength(); }

    friend Point operator+(const Point& p, const Vector& v) {
        return { p.x + v.x, p.y + v.y, p.z + v.z };
    }

    friend Point operator-(const Point& p, const Vector& v) {
        return { p.x - v.x, p.y - v.y, p.z - v.z };
    }

    friend Vector operator-(const Point& p, const Point& q) {
        return { p.x - q.x, p.y - q.y, p.z - q.z };
    }

    friend Point operator+(const Vector& v, const Point& p) { return p + v; }

    friend std::ostream& operator<<(std::ostream& os, const Point& p);
};

inline Point Point::nudgePoint(const Vector& v) const {
  return *this + ray_epsilon * v;
}

/**********************
 * Translation matrix *
 **********************/

class Matrix {
public: /* Methods: */

    Matrix () { }

    explicit Matrix (floating v) {
        for (size_t i = 0; i < 16; ++ i)
            m_data[i] = v;
    }

    Matrix (floating m00, floating m01, floating m02, floating m03,
            floating m10, floating m11, floating m12, floating m13,
            floating m20, floating m21, floating m22, floating m23,
            floating m30, floating m31, floating m32, floating m33)
        : m_data {
            m00, m10, m20, m30,
            m01, m11, m21, m31,
            m02, m12, m22, m32,
            m03, m13, m23, m33
        }
    { }

    floating& operator[](size_t i) { return m_data[i]; }
    floating operator[](size_t i) const { return m_data[i]; }

    floating& operator () (size_t i, size_t j) { return m_data[i + 4*j]; }
    floating operator () (size_t i, size_t j) const { return m_data[i + 4*j]; }

    Vector transform (Vector vec) const {
        auto result = Vector {0, 0, 0};
        const auto& self = *this;
        for (size_t i = 0; i < 3; ++ i) {
            for (size_t j = 0; i < 3; ++ j) {
                result[i] += vec[j]*self(i, j);
            }
        }

        return result;
    }

    Point transform (Point p) const {
        const auto& m = *this;
        const auto iw = 1.0 / (m(3, 0)*p[0] + m(3, 1)*p[1] + m(3,2)*p[2] + m(3,3));
        return {
            iw * (m(0, 0)*p[0] + m(0, 1)*p[1] + m(0, 2)*p[2] + m(0, 3)),
            iw * (m(1, 0)*p[0] + m(1, 1)*p[1] + m(1, 2)*p[2] + m(1, 3)),
            iw * (m(2, 0)*p[0] + m(2, 1)*p[1] + m(2, 2)*p[2] + m(2, 3))
        };
    }

    friend Matrix operator * (const Matrix& m1, const Matrix& m2) {
        auto result = Matrix {0};
        for (size_t r = 0; r < 4; ++ r)
            for (size_t c = 0; c < 4; ++ c)
                for (size_t i = 0; i < 4; ++ i)
                    result(r, c) += m1(r, i) * m2(i, c);
        return result;
    }

    // http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
    friend Matrix invert (const Matrix& m) {
        auto inv = Matrix {};

        inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
        inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
        inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
        inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
        inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
        inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
        inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
        inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
        inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
        inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
        inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
        inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
        inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
        inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
        inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
        inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

        const auto det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

        if (det == 0)
            return Matrix::identity ();

        const auto idet = 1.0 / det;

        Matrix result;
        for (size_t i = 0; i < 16; ++ i)
            result[i] = inv[i] * idet;

        return result;
    }

    static Matrix identity () {
        return { 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1 };
    }

    static Matrix scale (floating x, floating y, floating z) {
        return { x, 0, 0, 0,
                 0, y, 0, 0,
                 0, 0, z, 0,
                 0, 0, 0, 1 };
    }

    static Matrix scale (Vector vec) { return scale (vec[0], vec[1], vec[2]); }

    static Matrix translate (floating x, floating y, floating z) {
        return { 1, 0, 0, x,
                 0, 1, 0, y,
                 0, 0, 1, z,
                 0, 0, 0, 1 };
    }

    static Matrix translate (Vector vec) { return translate (vec[0], vec[1], vec[2]); }

    // http://www.opengl.org/sdk/docs/man2/xhtml/gluPerspective.xml
    static Matrix perspective (floating fov, floating asp, floating near, floating far) {
        const auto f = 1.0 / tan (fov * M_PI / 360.0);
        const auto d = 1.0 / (near - far);
        return { f/asp,  0, 0,                0,
                 0,    -f,  0,                0,
                 0,     0,  (near + far) * d, 2.0*near*far*d,
                 0,     0,  -1,               0
        };
    }

    // http://www.opengl.org/sdk/docs/man2/xhtml/gluLookAt.xml
    static Matrix lookAt (Point eye, Point center, Vector up) {
        up = normalised (up);
        const auto f = normalised (center - eye);
        const auto s = normalised (f.cross (up));
        const auto u = s.cross (f);
        const auto M = Matrix {
             s[0],  s[1],  s[2], 0,
             u[0],  u[1],  u[2], 0,
            -f[0], -f[1], -f[2], 0,
             0,     0,     0,    1
        };

        return M * Matrix::translate(-eye);
    }

private: /* Fields: */
    floating m_data[16];
};

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
    pix  = (uint32_t)(255.0 * clamp(r, 0.0, 1.0)) << 16;
    pix |= (uint32_t)(255.0 * clamp(g, 0.0, 1.0)) << 8;
    pix |= (uint32_t)(255.0 * clamp(b, 0.0, 1.0));
    return pix;
  }

  bool close(const Colour& c) const { return almost_zero(diff(c)); }

  bool isZero () const {
      return r*r + g*g + b*b == 0.0;
  }

  Colour& operator += (const Colour& c) {
    r += c.r; g += c.g; b += c.b;
    return *this;
  }

  Colour& operator *= (const Colour& c) {
    r *= c.r; g *= c.g; b *= c.b;
    return *this;
  }

  friend std::ostream& operator<<(std::ostream& os, const Colour& c);
};

inline Colour operator*(const Colour& c1, const Colour& c2) {
  return { c1.r * c2.r, c1.g * c2.g, c1.b * c2.b };
}

inline Colour operator*(const Colour& c1, floating s) {
  return { c1.r * s, c1.g * s, c1.b * s };
}

inline Colour operator/(const Colour& c1, floating s) {
  return { c1.r / s, c1.g / s, c1.b / s };
}

inline Colour operator+(const Colour& c1, const Colour& c2) {
  return { c1.r + c2.r, c1.g + c2.g, c1.b + c2.b };
}

inline Colour operator-(const Colour& c1, const Colour& c2) {
  return { c1.r - c2.r, c1.g - c2.g, c1.b - c2.b };
}

inline Colour operator*(floating s, const Colour& c2) {
  return { s * c2.r, s * c2.g, s * c2.b };
}

inline Colour expf(const Colour& c) {
  return { expf(c.r), expf(c.g), expf(c.b) };
}

inline floating luminance (floating r, floating g, floating b) {
    return 0.2126*r + 0.7152*g + 0.0722*b;
}

inline floating luminance (const Colour& c) {
    return luminance(c.r, c.g, c.b);
}

#endif
