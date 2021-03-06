#pragma once

#include "material.h"

#include <vector>

class Materials {
    using impl_t = std::vector<Material>;

public: /* Methods: */
    Materials()
        : m_materials()
    {
        m_materials.emplace_back(Colour{0, 0, 0}, 0.0, 0.0, 0.0, 1, 1);
    }

    material_index_t registerMaterial(Material mat) {
        material_index_t idx = m_materials.size();
        m_materials.push_back(mat);
        return idx;
    }

    const Material& operator[](material_index_t idx) const {
        return m_materials[idx];
    }

    static material_index_t lightMaterial() { return 0; }

    void shrink_to_fit() { impl_t(m_materials).swap(m_materials); }

private: /* Fields: */
    impl_t m_materials;
};
