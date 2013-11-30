#include "intersection.h"
#include "primitive.h"
#include "ray.h"

void Intersection::update(const Ray& ray, const Primitive* prim, floating dist) {
  if (dist > 0.0 && (!hasIntersections() || dist < this->dist())) {
      m_primitive = prim;
      m_dist = dist;
      m_point = ray.origin() + dist * ray.dir();
  }
}

std::ostream& operator<<(std::ostream& os, const Intersection& i) {
  os << "Intersection {";
  os << "point = " << i.m_point << ',';
  os << "dist = " << i.m_dist << ',';
  os << "primitive = ";
//  if (i.m_primitive != nullptr) {
//    os << *i.m_primitive;
//  } else {
//    os << "nullptr";
//  }
  os << '}';
  return os;
}
