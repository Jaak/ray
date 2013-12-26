#ifndef RAY_PRIMITIVE_H
#define RAY_PRIMITIVE_H

#include "geometry.h"
#include "material.h"
#include "texture.h"

class Intersection;
class Ray;
class Light;

class Primitive {
public: /* Methods: */

  explicit Primitive ()
    : m_material (0)
    , m_texture (-1)
    , m_light { nullptr }
  { }

  virtual ~Primitive() { }

  void setMaterial (material_index_t mat) { m_material = mat; }
  material_index_t material () const { return m_material; }

  void setTexture (texture_index_t texture) { m_texture = texture; }
  texture_index_t texture () const { return m_texture; }

  /// Emissive objects have a light source attached to them.
  bool emissive () const { return m_light != nullptr; }
  void setLight (Light* light) { m_light = light; }
  Light* getLight () const { return m_light; }

  /// Texture UV lookup at given coordinate.
  const virtual Colour getColourAtIntersection(const Point&, const Texture*) const {
    return {0.0, 0.0, 0.0};
  }

  /// Intersects ray with primitive on hit
  virtual void intersect(const Ray&, Intersection&) const = 0;

  /// Returns normal of the primitive on given position.
  virtual Vector normal(const Point&) const = 0;

  /// Least coordinate of the primitive on @a axis
  virtual floating getLeftExtreme(Axes axis) const = 0;

  /// Greates coordinate of the primitive on @a axis
  virtual floating getRightExtreme(Axes axis) const = 0;

private: /* Fields: */
  material_index_t m_material; ///< Material description of the primitive
  texture_index_t  m_texture; ///< Texture of the primitive
  Light*           m_light; ///< The primitive is emissive and thus has light attached.
};

#endif

