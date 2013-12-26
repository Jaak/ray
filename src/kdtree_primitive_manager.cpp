#include "kdtree_primitive_manager.h"

#include "primitive.h"
#include "ray.h"
#include "intersection.h"

#include <algorithm>
#include <cassert>

typedef const Primitive* PrimPtr;

/**
 * %Node of the tree.
 * Simply a binary tree which is kind of balanced.
 */
struct Node {
  floating   m_split;     ///< Splitting distance.
  Axes       m_axis;      ///< Splitting axis, is None is the node is leaf.
  Node*      m_left;      ///< Pointer to left subtree
  Node*      m_right;     ///< Pointer to right subtree
  PrimPtr    m_prims[1];  ///< List of primitives in the node

  Node ()
    : m_split { 0.0 }
    , m_axis { Axes::None }
    , m_left { nullptr }
    , m_right { nullptr }
    , m_prims { nullptr }
  { }

  bool empty() const { return m_prims[0] == nullptr; }

  size_t size() const {
    size_t i = 0;
    while (m_prims[i++])
      ;
    return i;
  }

  size_t treeSize() const {
    return size () + (m_left  ? m_left->treeSize()  : 0) +
               (m_right ? m_right->treeSize() : 0);
  }

  PrimPtr* prims () { return &m_prims[0]; }

  // TODO: return std::unique_ptr?
  static Node *make(const PrimList &prims = PrimList()) {
    size_t len = prims.size();
    void *memptr = malloc(sizeof(Node) + len * sizeof(PrimPtr));
    Node *out = new (memptr) Node();

    size_t i = 0;
    for (auto prim : prims) {
      out->m_prims[i++] = prim;
    }

    out->m_prims[i] = nullptr;
    return out;
  }

  static void release(Node *node) {
    if (node) {
      Node::release(node->m_left);
      Node::release(node->m_right);
      free(node);
    }
  }

  friend std::ostream &operator<<(std::ostream &o, const Node &) { return o; }
};

static unsigned long long icount = 0;

/**
 *
 */

struct StackElem {
  Point pb;
  Node* node;
  floating t;
  unsigned char prev;
};

/**
 *
 */

KdTreePrimitiveManager::KdTreePrimitiveManager()
  : m_root { nullptr }
{ }

KdTreePrimitiveManager::~KdTreePrimitiveManager() {
    for (auto prim : m_prims) {
        delete prim;
    }

    std::cout << "Intersections : " << icount << std::endl;
    Node::release (m_root);
}

bool intersectAabb(const Aabb& box, const Ray& ray, floating& a, floating& b) {
  const Point origin = ray.origin();
  const Vector dir = ray.dir();

  a = std::numeric_limits<floating>::min();
  b = std::numeric_limits<floating>::max();

  for (Axes i = Axes::X; i != Axes::None; ++i) {
    if (almost_zero (dir[i])) {
      if (origin[i] < box.m_p1[i] || origin[i] > box.m_p2[i]) {
        return false;
      }

      continue;
    }

    floating idir = 1.0 / dir[i];
    floating at = (box.m_p1[i] - origin[i]) * idir;
    floating bt = (box.m_p2[i] - origin[i]) * idir;

    if (at > bt)
      idir = at, at = bt, bt = idir;
    if (at > a)
      a = at;
    if (bt < b)
      b = bt;
    if (b < 0 || a > b)
      return false;
  }

  return true;
}

floating Aabb::split(Aabb& left, Aabb& right, Axes axis) const {
  left = right = *this;
  left.m_p2[axis] = (left.m_p1[axis] + left.m_p2[axis]) * 0.5;
  right.m_p1[axis] = left.m_p2[axis];
  return right.m_p1[axis];
}

void Aabb::split_at(Aabb& left, Aabb& right, Axes axis, floating pos) const {
  left = right = *this;
  left.m_p2[axis] = pos;
  right.m_p1[axis] = pos;
}

enum Side {
  LEFT = 0,
  RIGHT
};

struct Split {
  floating pos;
  Side side;
};

struct SplitCmp {
  inline bool operator()(const Split& l, const Split& r) const {
    return l.pos < r.pos;
  }
};

/**
 * Determins split position using surface area heuristic.
 * Tries to minimise function Area(left)*Objects(left) +
 * Area(right)*Objects(right).
 */
floating getOptimalSplitPosition(const PrimList& plist, Aabb const& box, Axes& axis) {
  const size_t len = plist.size ();
  std::vector<Split> splts(len * 2);
  Aabb left, right;
  size_t lc, rc, j;

  floating bestCost = std::numeric_limits<floating>::max();
  floating bestPos = -1.0;
  axis = Axes::X;

  for (Axes i = Axes::X; i != Axes::None; ++i) {
    j = 0;
    for (auto prim : plist) {
      splts[j].pos = prim->getLeftExtreme(i);
      splts[j++].side = LEFT;
      splts[j].pos = prim->getRightExtreme(i);
      splts[j++].side = RIGHT;
    }

    std::sort(splts.begin(), splts.end(), SplitCmp());

    lc = 0;
    rc = len;

    for (Split split : splts) {
      if (split.side == LEFT)
        ++lc;

      const auto pos = split.pos;
      box.split_at(left, right, i, pos);
      const auto cost = left.area() * lc + right.area() * rc;

      if (cost < bestCost) {
        bestCost = cost;
        bestPos = pos;
        axis = i;
      }

      if (split.side == RIGHT)
        --rc;
    }
  }

  return bestPos;
}

Node *buildKDTree(const PrimList &prims, const Aabb &box, int depth = 0) {
  if (prims.empty())
    return Node::make();

  if (depth >= 20)
    return Node::make(prims);

  Aabb leftBox, rightBox;
  Axes axis = Axes::None;
  const floating split = getOptimalSplitPosition(prims, box, axis);
  box.split_at(leftBox, rightBox, axis, split);

  if (leftBox.volume() <= epsilon || rightBox.volume() <= epsilon)
    return Node::make(prims);

  PrimList lefts, rights;
  for (auto p : prims) {
    if (p->getLeftExtreme(axis) < split) {
      lefts.push_back(p);
    }

    if (p->getRightExtreme(axis) >= split) {
      rights.push_back(p);
    }
  }

  Node *out = Node::make();
  out->m_split = split;
  out->m_axis = axis;
  out->m_left = buildKDTree(lefts, leftBox, depth + 1);
  out->m_right = buildKDTree(rights, rightBox, depth + 1);
  return out;
}

void KdTreePrimitiveManager::init() {
  if (m_prims.empty())
    return;

  for (Axes i = Axes::X; i != Axes::None; ++i) {
    m_bbox.m_p1[i] = m_prims.front()->getLeftExtreme(i);
    m_bbox.m_p2[i] = m_prims.front()->getRightExtreme(i);
  }

  for (auto prim : m_prims) {
    for (Axes i = Axes::X; i != Axes::None; ++i) {
      const floating l = prim->getLeftExtreme(i);
      const floating r = prim->getRightExtreme(i);
      m_bbox.m_p1[i] = fmin(m_bbox.m_p1[i], l);
      m_bbox.m_p2[i] = fmax(m_bbox.m_p2[i], r);
    }
  }

  m_root = buildKDTree(m_prims, m_bbox);
  std::cerr << "KD Tree built! Tree has " << m_root->treeSize() << " nodes."
            << std::endl;
}

void KdTreePrimitiveManager::addPrimitive(const Primitive *p) {
  assert(p != nullptr);
  m_prims.push_back(p);
}

/**
 * Implementation of TA^B_{rec}
 * Algorithm is described in http://www.cgg.cvut.cz/~havran/phdthesis.html
 */
Intersection KdTreePrimitiveManager::intersectWithPrims(const Ray& ray) const {
  StackElem stack[64];
  Node* far;
  Node* cur = m_root;
  floating a, b, t;
  int en = 0, ex = 1, tmp;

  if (!intersectAabb(m_bbox, ray, a, b)) {
    return {};
  }

  stack[en].t = a;
  stack[en].pb = ray.origin();

  if (a >= 0.0) {
    stack[en].pb += ray.dir() * a;
  }

  stack[ex].t = b;
  stack[ex].pb = ray.origin() + ray.dir() * b;
  stack[ex].node = nullptr;

  while (cur != nullptr) {
    while (cur->m_axis != Axes::None) {
      const float split = cur->m_split;
      Axes axis = cur->m_axis;

      if (stack[en].pb[axis] < split) {
        if (stack[ex].pb[axis] < split) {
          cur = cur->m_left;
          continue;
        }

        far = cur->m_right;
        cur = cur->m_left;
      } else {
        if (stack[ex].pb[axis] > split) {
          cur = cur->m_right;
          continue;
        }

        far = cur->m_left;
        cur = cur->m_right;
      }

      t = (split - ray.origin(axis)) / ray.dir(axis);
      tmp = ex;
      ++ex;
      if (ex == en)
        ++ex;

      stack[ex].prev = tmp;
      stack[ex].t = t;
      stack[ex].node = far;
      stack[ex].pb[axis] = split;

      axis = nextAxis(axis);
      stack[ex].pb[axis] = ray.origin(axis) + t * ray.dir(axis);

      axis = nextAxis(axis);
      stack[ex].pb[axis] = ray.origin(axis) + t * ray.dir(axis);
    }

    {
      Intersection intr;
      for (PrimPtr* ptr = cur->prims (); *ptr; ++ ptr, ++ icount) {
        auto prim = *ptr;
        prim->intersect(ray, intr);
        if (intr.hasIntersections()) {
          if (intr.dist() < stack[en].t - epsilon ||
              intr.dist() > stack[ex].t + epsilon) {
            intr.nullPrimitive();
          }
        }
      }

      if (intr.hasIntersections()) {
        return intr;
      }
    }

    en = ex;
    cur = stack[ex].node;
    ex = stack[en].prev;
  }

  return {};
}
