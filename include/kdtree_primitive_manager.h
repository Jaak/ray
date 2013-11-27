#ifndef RAY_KDTREE_PRIMITIVE_MANAGER_H
#define RAY_KDTREE_PRIMITIVE_MANAGER_H

#include "primitive_manager.h"
#include "geometry.h"
#include "aabb.h"

#include <vector>

typedef std::vector<const Primitive*> PrimList;

struct Node;
class Ray;

class KdTreePrimitiveManager : public PrimitiveManager {
public: /* Methods: */
  KdTreePrimitiveManager();
  ~KdTreePrimitiveManager();
  virtual void init();
  virtual void addPrimitive(const Primitive* p);
  virtual Intersection intersectWithPrims(const Ray& ray) const;

private:            /* Fields: */
  PrimList m_prims; ///< List of all the primitives.
  Node* m_root;     ///< Root of the kd-tree
  Aabb m_bbox;      ///< Scene box.
};

/**
 * @}
 */

#endif
