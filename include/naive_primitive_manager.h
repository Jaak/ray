#pragma once

#include "primitive_manager.h"

#include <vector>

class Primitive;
class Intersection;
class Ray;

/**
 * @ingroup PrimitiveManagers
 * Really naive primitive manager.
 * Stores primitives in a vector and intersects ray with every primitive.
 */
class NaivePrimitiveManager : public PrimitiveManager {
public: /* Methods: */
    ~NaivePrimitiveManager();

    void init() override {}

    void addPrimitive(const Primitive* p) override { m_prims.push_back(p); }

    void setSceneSphere(SceneSphere& sceneSphere) const override;

    Intersection intersectWithPrims(const Ray& ray) const override;

private: /* Fields: */
    std::vector<const Primitive*> m_prims;
};
