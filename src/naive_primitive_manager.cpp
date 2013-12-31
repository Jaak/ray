#include "naive_primitive_manager.h"

#include "aabb.h"
#include "intersection.h"
#include "primitive.h"
#include "ray.h"
#include "scene_sphere.h"

#include <iostream>

unsigned long long icount = 0;

NaivePrimitiveManager::~NaivePrimitiveManager() {
    for (auto p : m_prims) {
        delete p;
    }

    std::cout << "Intersections : " << icount << std::endl;
}

void NaivePrimitiveManager::setSceneSphere (SceneSphere& sceneSphere) const {
    Aabb bbox;

    for (size_t i = 0; i < 3; ++ i) {
        bbox.m_p1[i] = std::numeric_limits<floating>::max ();
        bbox.m_p2[i] = std::numeric_limits<floating>::min ();
    }

    for (const auto p : m_prims) {
        for (size_t i = 0; i < 3; ++ i) {
            bbox.m_p1[i] = std::min (bbox.m_p1[i], p->getLeftExtreme (i));
            bbox.m_p2[i] = std::min (bbox.m_p2[i], p->getRightExtreme (i));
        }
    }

    const auto vecToMax = bbox.m_p2 - bbox.m_p1;
    const auto middlePoint = bbox.m_p1 + 0.5*vecToMax;
    sceneSphere.setCenter (middlePoint);
    sceneSphere.setRadius (0.5*vecToMax.length ());
}

Intersection NaivePrimitiveManager::intersectWithPrims(const Ray& ray) const {
  Intersection intr;
  for (auto p : m_prims) {
    ++icount;
    p->intersect(ray, intr);
  }

  return intr;
}
