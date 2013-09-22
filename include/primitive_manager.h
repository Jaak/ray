#ifndef RAY_PRIMITIVE_MANAGER_H
#define RAY_PRIMITIVE_MANAGER_H

class Intersection;
class Primitive;
class Ray;

#include <ostream>

/**
 * Base class for all primitive-managers.
 * Takes care of
 * 	- storing primitives
 * 	- deallocating primitves
 * 	- intersecting ray with primitives
 *
 * Note that this all could be done statically by making scene a template
 * class. This approach would be faster (wins a function call and a vtable
 * lookup per ray intersection patch).
 */
class PrimitiveManager {
public: /* Methods: */
  PrimitiveManager() {}
  virtual ~PrimitiveManager() {}

  /// Add a primitive to primitive manager.
  virtual void addPrimitive(Primitive* prim) = 0;

  /**
   * Initialises the primitive manager.
   * After this call no primitives should be added to the
   * manager, otherwise managers behaviour is undefined.
   */
  virtual void init() = 0;

  /// Intersects @a ray with primitives.
  virtual Intersection intersectWithPrims(const Ray& ray) const = 0;

  friend std::ostream& operator<<(std::ostream& o, const PrimitiveManager&) {
    return o;
  }
};

#endif
