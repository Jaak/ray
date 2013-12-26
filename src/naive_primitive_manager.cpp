#include "naive_primitive_manager.h"

#include "ray.h"
#include "intersection.h"
#include "primitive.h"

unsigned long long icount = 0;

NaivePrimitiveManager::~NaivePrimitiveManager() {
    for (auto p : m_prims) {
        delete p;
    }

    std::cout << "Intersections : " << icount << std::endl;
}

Intersection NaivePrimitiveManager::intersectWithPrims(const Ray& ray) const {
  Intersection intr;
  for (auto p : m_prims) {
    ++icount;
    p->intersect(ray, intr);
  }

  return intr;
}
