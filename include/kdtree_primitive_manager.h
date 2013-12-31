#ifndef RAY_KDTREE_PRIMITIVE_MANAGER_H
#define RAY_KDTREE_PRIMITIVE_MANAGER_H

#include "primitive_manager.h"
#include "aabb.h"

#include <vector>

using PrimList = std::vector<const Primitive*>;

struct Node;
class Ray;
class SceneSphere;

class KdTreePrimitiveManager : public PrimitiveManager {
public: /* Methods: */
  KdTreePrimitiveManager();
  ~KdTreePrimitiveManager();

  void init() override;
  void addPrimitive(const Primitive* p) override;
  void setSceneSphere (SceneSphere& sceneSphere) const override;
  Intersection intersectWithPrims(const Ray& ray) const override;
  void debugDrawOnFramebuffer (const Camera& cam, Framebuffer& buf) const override;

private:            /* Fields: */
  PrimList m_prims; ///< List of all the primitives.
  Node* m_root;     ///< Root of the kd-tree
  Aabb m_bbox;      ///< Scene box.
};

/**
 * @}
 */

#endif
