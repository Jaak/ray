#ifndef RAY_NAIVE_PRIMITIVE_MANAGER_H
#define RAY_NAIVE_PRIMITIVE_MANAGER_H

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

	void init() { }
	
    void addPrimitive(const Primitive* p) {
		m_prims.push_back(p);
	}

	Intersection intersectWithPrims(const Ray& ray) const;

private: /* Fields: */
    std::vector<const Primitive* > m_prims;
};

#endif
