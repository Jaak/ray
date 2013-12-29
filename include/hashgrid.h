#ifndef RAY_HASHGRID_H
#define RAY_HASHGRID_H

#include "geometry.h"

#include <cassert>

// Original source:
//  "Optimized Spatial Hashing for Collision Detection of Deformable Objects"
class HashGrid {
private: /* Types: */

    using index_t = uint32_t;

public: /* Methods: */

    HashGrid (const HashGrid&) = delete;
    HashGrid& operator = (const HashGrid&) = delete;

    HashGrid ()
        : m_sqrRadius {0.0}
        , m_invCellSize {0.0}
    { }


    // Iter::value_type must have method "position" returning a Point defined.
    template <typename Iter>
    void build (Iter begin, Iter end, size_t numCells, floating radius) {
        assert (radius != 0.0);
        assert (numCells <= std::numeric_limits<index_t>::max ());
        assert (std::distance (begin, end) <= std::numeric_limits<index_t>::max ());

        m_indices.assign (std::distance (begin, end), 0);
        m_cellEnds.assign (numCells, 0);
        m_sqrRadius = radius * radius;
        m_invCellSize = 1.0 / (2.0 * radius);

        for (size_t i = 0; i < 3; ++ i) {
            m_minCoord[i] = std::numeric_limits<floating>::max ();
        }

        for (auto i = begin; i != end; ++ i) {
            const auto pos = i->position ();
            for (size_t j = 0; j < 3; ++ j) {
                m_minCoord[j] = fmin (m_minCoord[j], pos[j]);
            }
        }

        for (auto i = begin; i != end; ++ i) {
            ++ m_cellEnds[hash (i->position ())];
        }

        index_t sum = 0;
        for (size_t i = 0; i < m_cellEnds.size (); ++ i) {
            const auto temp = m_cellEnds[i];
            m_cellEnds[i] = sum;
            sum += temp;
        }

        index_t offset = 0; // numeric offset/distance from begin
        for (auto i = begin; i != end; ++ i, ++ offset) {
            const auto pos = i->position ();
            const auto targetIdx = m_cellEnds[hash (pos)] ++;
            m_indices[targetIdx] = offset;
        }
    }

    template <typename Iter, typename F>
    void visit (Iter begin, Iter end, Point point, F visitor) const {
        const auto distMin = point - m_minCoord;
        const auto coordF = m_invCellSize * distMin;

        const index_t px = (index_t) std::floor (coordF[0]);
        const index_t py = (index_t) std::floor (coordF[1]);
        const index_t pz = (index_t) std::floor (coordF[2]);
        const index_t pxo = px + (coordF[0] - px < 0.5 ? -1 : 1);
        const index_t pyo = py + (coordF[1] - py < 0.5 ? -1 : 1);
        const index_t pzo = pz + (coordF[2] - pz < 0.5 ? -1 : 1);
        const index_t coords[8][3] = {
              {px , py , pz }, {px , py , pzo}, {px , pyo, pz }, {px , pyo, pzo}
            , {pxo, py , pz }, {pxo, py , pzo}, {pxo, pyo, pz }, {pxo, pyo, pzo}
        };

        size_t hits = 0;
        for (size_t i = 0; i < 8; ++ i) {
            const auto idx = hash (coords[i][0], coords[i][1], coords[i][2]);
            for (size_t j = cellBegin(idx); j < cellEnd(idx); ++ j) {
                const auto iter = begin + m_indices[j];
                const auto particlePos = iter->position ();
                const auto sqrDist = (point - particlePos).sqrlength ();
                if (sqrDist <= m_sqrRadius) {
                    ++ hits;
                    visitor (*iter);
                }
            }
        }
    }

private:

    index_t hash (index_t x, index_t y, index_t z) const {
        // Just few prime numbers;
        static constexpr index_t P1 = 73856093u;
        static constexpr index_t P2 = 19349663u; // stupidly, this one isn't a prime...
        static constexpr index_t P3 = 83492791u;
        const index_t MOD = m_cellEnds.size ();
        return ((x * P1) ^ (y * P2) ^ (z * P3)) % MOD;
    }

    index_t hash (Point point) const {
        const auto distMin = point - m_minCoord;
        const auto coordF = m_invCellSize * distMin;
        return hash (
            (index_t) std::floor (coordF[0]),
            (index_t) std::floor (coordF[1]),
            (index_t) std::floor (coordF[2])
        );
    }

    index_t cellBegin (size_t idx) const {
        return idx ? m_cellEnds[idx - 1] : 0;
    }

    index_t cellEnd (size_t idx) const {
        return m_cellEnds[idx];
    }

private: /* Fields: */
    Point m_minCoord;
    std::vector<index_t> m_indices;
    std::vector<index_t> m_cellEnds;
    floating m_sqrRadius;
    floating m_invCellSize;
};

#endif
