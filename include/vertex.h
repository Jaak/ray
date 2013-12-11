#ifndef RAY_VERTEX_H
#define RAY_VERTEX_H

#include "geometry.h"
#include <vector>

class Primitive;


enum EventType {
    DIFFUSE,
    REFLECT,
    REFRACT,
    EYE
};

struct Vertex {
  Point            m_pos;
  Vector           m_normal;
  Colour           m_col; // TODO: use proper BRDF here
  const Primitive* m_prim;
  floating         m_pr;
  EventType        m_event;

  Vertex (const Point& pos, const Vector& normal, const Colour& col, const Primitive* prim, floating pr, EventType event)
      : m_pos {pos}
      , m_normal {normal}
      , m_col {col}
      , m_prim {prim}
      , m_pr {pr}
      , m_event {event}
  { }
};

using VertexList = std::vector<Vertex>;

#endif /* RAY_VERTEX_H */
