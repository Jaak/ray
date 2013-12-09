#ifndef RAY_PRIMITIVE_H
#define RAY_PRIMITIVE_H

#include "geometry.h"
#include "material.h"
#include "texture.h"

class Intersection;
class Ray;

class Primitive {
public: /* Methods: */

  explicit Primitive (bool is_light = false)
    : m_material (0)
    , m_texture (-1)
    , m_is_light (is_light)
  { }

  virtual ~Primitive() { }

  const virtual Colour getColourAtIntersection(const Point&, const Texture*) const {
    return Colour(0.0, 0.0, 0.0);
  }

  void setMaterial (material_index_t mat) {
    m_material = mat;
  }

  material_index_t material () const {
    return m_material;
  }

  void setTexture (texture_index_t texture) {
    m_texture = texture;
  }

  texture_index_t texture () const {
    return m_texture;
  }

  /// Intersects ray with primitive on hit
  virtual void intersect(const Ray&, Intersection&) const = 0;

  /// Returns normal of the primitive on given position.
  virtual Vector normal(const Point&) const = 0;

  /// Outputs primitive description into stream.
  // virtual void output(std::ostream&) const { };

  /// Least coordinate of the primitive on @a axis
  virtual floating getLeftExtreme(Axes axis) const = 0;

  bool is_light() const { return m_is_light; }

  /// Greates coordinate of the primitive on @a axis
  virtual floating getRightExtreme(Axes axis) const = 0;

//  friend std::ostream& operator << (std::ostream& o, const Primitive& p) {
//    p.output(o);
//    return o;
//  }

private: /* Fields: */
  material_index_t m_material; ///< Material description of the primitive
  texture_index_t  m_texture; ///< Texture of the primitive
  const bool       m_is_light; ///< If the given primitive is emitting light.
};

#endif

