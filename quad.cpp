
#include <assert.h>
#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <unordered_set>
#include <variant>

#include "gfx.h"

#include "glad/glad.h"

static Mesh mesh_cross, mesh_quad;
static Shader shader;

template <class T>
struct TreeNode {
  vec2 pos;
  T data;

  void draw() {
    shader.set_uniform("pos", pos);
    shader.set_uniform("sz", 0.02f * vec2{0.09, 0.16});
    shader.set_uniform("col", vec4{0.8, 0.2, 0.7, 1});
    mesh_quad.draw();
  }
};

template <class T>
struct QuadTree;

template <class T>
inline size_t hash_combine(size_t seed, T const& v) {
  return seed ^ (std::hash<T>(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

template <class H, class... T>
inline size_t hash_combine(size_t seed, H const& h, T const&... t) {
  return hash_combine(hash_combine(seed, h), t...);
}

template <class T, int n>
struct std::hash<vec<T, n>> {
  size_t operator()(vec<T, n> v) const noexcept {
    return fold([](size_t a, T b) { return hash_combine(a, b); }, size_t(n),
                vec3{1, 2, 3});
  }
};

template <class T>
struct std::hash<TreeNode<T>> {
  size_t operator()(const TreeNode<T>& q) const noexcept {
    return hash_combine(0, q.pos, q.dat);
  }
};

template <class T>
struct std::hash<QuadTree<T>> {
  size_t operator()(const QuadTree<T>& q) const noexcept {
    return hash_combine(0, q.pos, q.dat, q.children);
  }
};

template <class T>
struct QuadTree {
  vec2 pos;
  vec2 size;
  QuadTree<T>* parent;
  std::variant<std::monostate,
               std::array<std::unique_ptr<QuadTree<T>>, 4>,
               TreeNode<T>>
      children;

  QuadTree(QuadTree<T>* parent = 0, vec2 pos = {}, vec2 size = {1, 1})
      : parent(parent), pos(pos), size(size), children(std::monostate{}) {}

  bool is_split() {
    return std::holds_alternative<std::array<std::unique_ptr<QuadTree<T>>, 4>>(
        children);
  }
  bool is_leaf() { return std::holds_alternative<TreeNode<T>>(children); }
  bool is_none() { return std::holds_alternative<std::monostate>(children); }

  TreeNode<T>* get_leaf() {
    if (is_leaf()) {
      return &std::get<TreeNode<T>>(children);
    }
    return 0;
  }

  std::array<std::unique_ptr<QuadTree<T>>, 4>* get_split() {
    if (is_split()) {
      return &std::get<std::array<std::unique_ptr<QuadTree<T>>, 4>>(children);
    }
    return 0;
  }

  int get_quadrant(vec2 v) {
    int x = v.x < pos.x;
    int y = v.y < pos.y;
    int q = (x ^ y) + 2 * y;
    if (!(q < 4 && q >= 0)) {
      printf("%d\n", q);
      assert(false);
    }
    return q;
  }

  void erase() {
    children = std::monostate{};
    if (!parent)
      return;

    bool has_children = false;
    for (auto& s : *parent->get_split())
      has_children |= !s->is_none();
    if (!has_children)
      parent->erase();
  }

  void find(vec2 p, vec2 s, std::unordered_set<QuadTree<T>*>& collection) {
    if (is_none()) {
      return;
    }

    if (auto c = get_leaf()) {
      if (c->pos.x <= p.x + s.x && c->pos.x >= p.x - s.x &&
          c->pos.y <= p.y + s.y && c->pos.y >= p.y - s.y) {
        collection.insert(this);
      }
      return;
    }

    for (auto& c : *get_split())
      c->find(p, s, collection);
  }

  bool insert(T data, vec2 v) {
    if (auto c = get_split()) {
      return (*c)[get_quadrant(v)]->insert(data, v);
    }

    if (auto c = get_leaf()) {
      vec2 sz = size * 0.5f;
      vec2 q0 = pos + sz;
      vec2 q1 = pos + vec2{-sz.x, +sz.y};
      vec2 q2 = pos + vec2{-sz.x, -sz.y};
      vec2 q3 = pos + vec2{+sz.x, -sz.y};

      auto node = std::move(*c);
      if (len(node.pos - v) < 0.001f) {
        return true;
      }

      children = std::array<std::unique_ptr<QuadTree<T>>, 4>{
          std::make_unique<QuadTree<T>>(this, q0, sz),
          std::make_unique<QuadTree<T>>(this, q1, sz),
          std::make_unique<QuadTree<T>>(this, q2, sz),
          std::make_unique<QuadTree<T>>(this, q3, sz)};

      return insert(std::move(node.data), node.pos) &&
             insert(std::move(data), v);
    }

    children = TreeNode<T>{v, std::move(data)};
    return true;
  }

  void draw() {
    if (is_none()) {
      return;
    }

    if (is_split()) {
      shader.set_uniform("pos", pos);
      shader.set_uniform("sz", size);
      shader.set_uniform("col", vec4{1, 1, 1, 1});
      mesh_cross.draw();
      for (auto& q :
           std::get<std::array<std::unique_ptr<QuadTree<T>>, 4>>(children)) {
        q->draw();
      }
    } else {
      std::get<TreeNode<T>>(children).draw();
    }
  }
};

int main() {
  QuadTree<int>* tree = new QuadTree<int>{};

  Window win("quadtree");

  shader.load(
#include "main.vert"
      ,
#include "main.frag"
  );

  vec2 v[4] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
  vec2 u[4] = {{-1, -1}, {-1, 1}, {1, 1}, {1, -1}};

  Mesh::AttribLayout layout = {0, 2, 8, 0};
  mesh_cross.create(v, sizeof v, 4, &layout, 1, Primitive::Lines);
  mesh_quad.create(u, sizeof u, 4, &layout, 1, Primitive::Quads);

  double t = 0;

  while (win.poll()) {
    t += win.dt;
    shader.bind();
    if (win.get_mouse_button(1)) {
      shader.set_uniform("pos", win.mnorm);
      shader.set_uniform("sz", vec2{1.f, 1.f} / 8.f);
      shader.set_uniform("col", vec4{0.2, 0.6, 0.7, 0.1});
      mesh_quad.draw();
      std::unordered_set<QuadTree<int>*> nodes;
      tree->find(win.mnorm, vec2{1.f, 1.f} / 8.f, nodes);
      for (auto n : nodes) {
        n->erase();
      }
    }

    if (t > 0.01 && win.get_mouse_button(0)) {
      t = 0;
      assert(tree->insert(0, win.mnorm));
    }

    tree->draw();
  }
}