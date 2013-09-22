#include "ray.h"

#include "primitive.h"
#include "intersection.h"

Ray Ray::reflect(const Intersection& intr) const {
  const Point P = intr.point();
  const Vector V = m_dir;
  Vector N = normal(intr);
  if (intr.type() == Intersection::Type::INTERNAL) {
    N = -N;
  }

  const Vector R = normalised(V - (2 * V.dot(N)) * N);
  return { P + R * ray_epsilon, R };
}

Vector Ray::normal(const Intersection& intr) const {
  if (!intr.hasIntersections()) {
    return { 0, 0, 0 };
  }

  return intr.getPrimitive()->normal(intr.point());
}
