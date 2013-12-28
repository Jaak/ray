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

    template <typename Iter>
    HashGrid (Iter begin, Iter end, size_t numCells, floating radius)
        : m_indices (std::distance (begin, end), 0)
        , m_cellBegins (numCells + 1, 0)
        , m_sqrRadius {radius * radius}
        , m_invCellSize {1.0 / (2.0 * radius)}
    {
        assert (numCells <= std::numeric_limits<index_t>::max ());
        assert (std::distance (begin, end) <= std::numeric_limits<index_t>::max ());

        for (size_t i = 0; i < 3; ++ i) {
            m_minCoord[i] = std::numeric_limits<floating>::max ();
        }

        for (Iter i = begin; i != end; ++ i) {
            const auto pos = i->position ();
            ++ m_cellBegins[getCellIndex (pos)];
            for (size_t j = 0; j < 3; ++ j) {
                m_minCoord[j] = fmin (m_minCoord[j], pos[j]);
            }
        }

        index_t sum = 0;
        for (index_t& cur : m_cellBegins) {
            const auto temp = cur;
            cur = sum;
            sum += temp;
        }

        index_t offset = 0; // numeric offset/distance from begin
        for (Iter i = begin; i != end; ++ i, ++ offset) {
            const auto pos = i->position ();
            const auto targetIdx = m_cellBegins[getCellIndex (pos)];
            m_indices[targetIdx] = offset;
        }
    }

    template <typename Iter, typename F>
    void visit (Iter begin, Point pos, F visitor) const {
        const auto distMin = pos - m_minCoord;

        for (size_t i = 0; i < 3; ++ i) {
            // sanity check
            assert (0 <= distMin[i]);
        }

        const auto cellPt = m_invCellSize * distMin;

        floating fx, fy, fz;
        const index_t px = (index_t) modf (cellPt[0], &fx);
        const index_t py = (index_t) modf (cellPt[0], &fy);
        const index_t pz = (index_t) modf (cellPt[0], &fz);
        const index_t pxo = px + (fx < 0.5 ? -1 : 1);
        const index_t pyo = py + (fy < 0.5 ? -1 : 1);
        const index_t pzo = pz + (fz < 0.5 ? -1 : 1);
        const index_t coords[8][3] = {
              {px , py , pz }, {px , py , pzo}, {px , pyo, pz }, {px , pyo, pzo}
            , {pxo, py , pz }, {pxo, py , pzo}, {pxo, pyo, pz }, {pxo, pyo, pzo}
        };

        for (size_t i = 0; i < 8; ++ i) {
            const auto idx = getCellIndex(coords[i][0], coords[i][1], coords[i][2]);
            for (size_t j = m_cellBegins[idx]; j < m_cellBegins[idx + 1]; ++ j) {
                const auto iter = begin + m_indices[j];
                const auto particlePos = iter->position ();
                const auto sqrDist = (pos - particlePos).sqrlength ();
                if (sqrDist <= m_sqrRadius)
                    visitor (*iter);
            }
        }
    }

private:

    index_t getCellIndex (index_t x, index_t y, index_t z) const {
        // Just few prime numbers;
        static constexpr index_t P1 = 73856093u;
        static constexpr index_t P2 = 19349663u; // stupidly, this one isn't a prime...
        static constexpr index_t P3 = 83492791u;
        const index_t MOD = m_cellBegins.size () - 1; // one extra!
        return ((x * P1) ^ (y * P2) ^ (z * P3)) % MOD;
    }

    index_t getCellIndex (Point point) const {
        const auto distMin = point - m_minCoord;
        const auto coordF = m_invCellSize * distMin;
        return getCellIndex (
            (index_t) std::floor (coordF[0]),
            (index_t) std::floor (coordF[1]),
            (index_t) std::floor (coordF[2])
        );
    }

private: /* Fields: */
    Point m_minCoord;
    std::vector<index_t> m_indices;
    std::vector<index_t> m_cellBegins;

    const floating m_sqrRadius;
    const floating m_invCellSize;
};

#endif
