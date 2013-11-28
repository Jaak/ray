#ifndef RAY_TABLE_H
#define RAY_TABLE_H

#include <vector>
#include <cassert>

// Simply a table of values. Use tbl(x, y) for indexing.
template <typename T>
class table : std::vector<T> {
public: /* Methods: */

    table (size_t width, size_t height, T def = T {})
        : std::vector<T>(width*height, def)
        , m_width { width }
        , m_height { height }
    { }

    T& operator () (size_t i, size_t j) {
        assert (i < m_width && j < m_height);
        return (*this)[i*m_height + j];
    }

    const T& operator () (size_t i, size_t j) const {
        assert (i < m_width && j < m_height);
        return (*this)[i*m_height + j];
    }

private: /* Fields: */
    const size_t m_width;
    const size_t m_height;
};

#endif /* RAY_TABLE_H */
