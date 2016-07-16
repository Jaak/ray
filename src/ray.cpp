#include "ray.h"

#include "intersection.h"
#include "primitive.h"

#include <cassert>

Ray Ray::reflect(const Intersection& intr) const {
    assert(intr.getPrimitive());
    const auto P = intr.point();
    auto       N = intr.getPrimitive()->normal(P);
    const auto V = m_dir;
    const auto internal = V.dot(N) < 0.0;
    if (internal) {
        N = -N;
    }

    const auto R = normalised(V - (2 * V.dot(N)) * N);
    return {P + R * ray_epsilon, R};
}
