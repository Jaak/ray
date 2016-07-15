#pragma once

class Colour;

class Surface {
public: /* Methods: */

    virtual ~Surface() {}

    int height() const { return m_height; }
    int width() const { return m_width; }

    void setDimensions(int h, int w) {
        m_height = h;
        m_width = w;
    }

    virtual void init() = 0;
    virtual void setPixel(int, int, const Colour&) = 0;

protected: /* Fields: */
    int m_height, m_width;
};
