
#include <assert.h>
#include <array>
#include <iostream>
#include <memory>
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
    shader.set_uniform("col", vec3{0.8, 0.2, 0.7});
    mesh_quad.draw();
  }
};

template <class T>
struct QuadTree {
  vec2 pos = {};
  vec2 size = {1, 1};

  QuadTree(vec2 pos = {}, vec2 size = {1, 1}) : pos(pos), size(size) {}

  std::variant<std::monostate,
               std::array<std::unique_ptr<QuadTree<T>>, 4>,
               TreeNode<T>>
      children = std::monostate{};

  bool is_split() {
    return std::holds_alternative<std::array<std::unique_ptr<QuadTree<T>>, 4>>(
        children);
  }

  bool is_leaf() { return std::holds_alternative<TreeNode<T>>(children); }
  bool is_none() { return std::holds_alternative<std::monostate>(children); }

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

  bool insert(T data, vec2 v) {
    if (is_split()) {
      return std::get<std::array<std::unique_ptr<QuadTree<T>>, 4>>(
                 children)[get_quadrant(v)]
          ->insert(data, v);
    }

    if (is_leaf()) {
      vec2 sz = size * 0.5f;
      vec2 q0 = pos + sz;
      vec2 q1 = pos + vec2{-sz.x, +sz.y};
      vec2 q2 = pos + vec2{-sz.x, -sz.y};
      vec2 q3 = pos + vec2{+sz.x, -sz.y};

      auto node = std::get<TreeNode<T>>(children);
      if (len(node.pos - v) < 0.001f) {
        return true;
      }

      children = std::array<std::unique_ptr<QuadTree<T>>, 4>{
          std::make_unique<QuadTree<T>>(q0, sz),
          std::make_unique<QuadTree<T>>(q1, sz),
          std::make_unique<QuadTree<T>>(q2, sz),
          std::make_unique<QuadTree<T>>(q3, sz)};

      return insert(node.data, node.pos) && insert(data, v);
    }

    children = TreeNode<T>{v, data};
    return true;
  }

  void draw() {
    if (is_none()) {
      return;
    }

    if (is_split()) {
      shader.set_uniform("pos", pos);
      shader.set_uniform("sz", size);
      shader.set_uniform("col", vec3{1, 1, 1});
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
  QuadTree<int> tree;

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
    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR);

    t += win.dt;

    if (win.get_mouse_button(1)) {
      tree = QuadTree<int>{};
    }

    if (t > 0.001 && win.get_mouse_button(0)) {
      t = 0;

      if (win.mnorm.x < -1)
        win.mnorm.x = -1;
      if (win.mnorm.x > +1)
        win.mnorm.x = +1;
      if (win.mnorm.y < -1)
        win.mnorm.y = -1;
      if (win.mnorm.y > +1)
        win.mnorm.y = +1;

      assert(tree.insert(0, win.mnorm));
    }

    shader.bind();
    tree.draw();
  }
}