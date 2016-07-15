#pragma once

#include <cassert>
#include <vector>

// Simply a table of values. Use tbl(x, y) for indexing.
template <typename T>
class table : std::vector<T> {
public: /* Methods: */

    table(size_t width, size_t height, T def = T{})
        : std::vector<T>(width * height, def)
        , m_width{width}
        , m_height{height}
    {}

    size_t height() const { return m_height; }
    size_t width() const { return m_width; }

    T& operator()(size_t i, size_t j) {
        assert(i < m_width && j < m_height);
        return (*this)[i * m_height + j];
    }

    const T& operator()(size_t i, size_t j) const {
        assert(i < m_width && j < m_height);
        return (*this)[i * m_height + j];
    }

    void fill(const T& value) {
        std::fill(this->begin(), this->end(), value);
    }

    using std::vector<T>::begin;
    using std::vector<T>::end;

private: /* Fields: */
    const size_t m_width;
    const size_t m_height;
};
