
#include <assert.h>
#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <unordered_set>
#include <variant>
#include <vector>

#include "gfx.h"

#include "glad/glad.h"

static Mesh mesh_cross, mesh_quad;
static Shader shader;

struct TreeNode {
  vec2 pos = {0, 0};
  vec2 vel = {0, 0};
  vec2 draw(f32 dt) {
    shader.set_uniform("pos", pos);
    shader.set_uniform("sz", 0.02f * vec2{0.09, 0.16});
    shader.set_uniform("col", vec4{0.8, 0.2, 0.7, 1});
    mesh_quad.draw();
    return pos += vel * dt;
  }
};

struct Range {
  vec2 lo, hi;
  bool contains(Range r) const {
    return lo.x < r.lo.x && lo.y < r.lo.y && hi.x > r.hi.x && hi.y > r.hi.y;
  }

  bool overlaps(Range r) const {
    return lo.x > r.lo.x && lo.x < r.hi.x || hi.x > r.lo.x && hi.x < r.hi.x ||
           lo.y > r.lo.y && lo.y < r.hi.y || hi.y > r.lo.y && hi.y < r.hi.y;
  }
};

struct Rect {
  vec2 pos = {0, 0};
  vec2 size = {0, 0};

  Range range() const { return {pos - size * 0.5f, pos + size * 0.5f}; }

  bool contains(Rect r) const { return range().contains(r.range()); }
  bool contains(vec2 v) const { return contains(Rect{v, v}); }
  bool overlaps(Rect r) const { return range().overlaps(r.range()); }

  void divide(Rect r[4]) const {
    vec2 sz = size * 0.5f;
    r[0] = {pos + sz, sz};
    r[1] = {pos + vec2{-sz.x, +sz.y}, sz};
    r[2] = {pos + vec2{-sz.x, -sz.y}, sz};
    r[3] = {pos + vec2{+sz.x, -sz.y}, sz};
  }
};

struct QuadTree {
  Rect rect;
  QuadTree* parent;
  std::variant<std::monostate,
               std::array<std::unique_ptr<QuadTree>, 4>,
               TreeNode>
      children;

  QuadTree(QuadTree* parent = 0, Rect rect = {{0, 0}, {2, 2}})
      : parent(parent), rect(rect), children(std::monostate{}) {}

  bool is_split() {
    return std::holds_alternative<std::array<std::unique_ptr<QuadTree>, 4>>(
        children);
  }

  bool is_leaf() { return std::holds_alternative<TreeNode>(children); }
  bool is_none() { return std::holds_alternative<std::monostate>(children); }

  TreeNode* get_leaf() {
    if (is_leaf()) {
      return &std::get<TreeNode>(children);
    }
    return 0;
  }

  std::array<std::unique_ptr<QuadTree>, 4>* get_split() {
    if (is_split()) {
      return &std::get<std::array<std::unique_ptr<QuadTree>, 4>>(children);
    }
    return 0;
  }

  int get_quadrant(vec2 v) {
    int x = v.x < rect.pos.x;
    int y = v.y < rect.pos.y;
    int q = (x ^ y) + 2 * y;
    if (!(q < 4 && q >= 0)) {
      assert(false);
    }
    return q;
  }

  bool has_children() {
    bool re = false;
    if (auto c = get_split())
      for (auto& c : *c)
        re |= !c->is_none();
    return re;
  }

  void erase() {
    children = std::monostate{};
    if (parent && !parent->has_children())
      parent->erase();
    return;
  }

  void collect(std::unordered_set<QuadTree*>& collection, int& counter) {
    counter++;
    if (auto c = get_leaf()) {
      collection.insert(this);
      return;
    }

    if (auto c = get_split())
      for (auto& c : *c)
        c->collect(collection, counter);
  }

  void find(Rect r, std::unordered_set<QuadTree*>& collection, int& counter) {
    counter++;
    if (is_none())
      return;

    if (r.contains(rect)) {
      collect(collection, counter);
      return;
    }

    if (!r.overlaps(rect)) {
      return;
    }

    if (auto c = get_leaf()) {
      if (r.contains(c->pos))
        collection.insert(this);
      return;
    }

    for (auto& c : *get_split())
      c->find(r, collection, counter);
  }

  int num() {
    int re = 1;
    if (auto c = get_split())
      for (auto& c : *c)
        re += c->num();
    return re;
  }

  bool insert(TreeNode v) {
    if (auto c = get_split()) {
      return (*c)[get_quadrant(v.pos)]->insert(v);
    }

    if (auto c = get_leaf()) {
      auto node = *c;

      if (len(node.pos - v.pos) < 0.00001f)
        return false;
      
      Rect r[4];
      rect.divide(r);
      children = std::array<std::unique_ptr<QuadTree>, 4>{
          std::make_unique<QuadTree>(this, r[0]),
          std::make_unique<QuadTree>(this, r[1]),
          std::make_unique<QuadTree>(this, r[2]),
          std::make_unique<QuadTree>(this, r[3])};

      insert(node);
      insert(v);
      return true;
    }

    children = v;
    return true;
  }

  void draw(f32 dt, std::vector<TreeNode>& reinsert) {
    if (is_none()) {
      return;
    }

    if (auto c = get_split()) {
      if (!has_children()) {
        children = std::monostate{};
        return;
      }
      shader.set_uniform("pos", rect.pos);
      shader.set_uniform("sz", rect.size);
      shader.set_uniform("col", vec4{1, 1, 1, 1});
      mesh_cross.draw();

      for (auto& c : *c)
        c->draw(dt, reinsert);
      return;
    }

    if (!rect.contains(get_leaf()->draw(dt))) {
      reinsert.push_back(*get_leaf());
      children = std::monostate{};
    }
  }
};

int main() {
  QuadTree* tree = new QuadTree{};

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
      shader.set_uniform("sz", 0.5f * vec2{1.f, 1.f} / 8.f);
      shader.set_uniform("col", vec4{0.2, 0.6, 0.7, 0.1});
      mesh_quad.draw();
      int counter = 0;

      std::unordered_set<QuadTree*> nodes;
      tree->find({win.mnorm, vec2{1.f, 1.f} / 8.f}, nodes, counter);
      printf("Took %d iterations with %d nodes\n", counter, tree->num());
      // for (auto n : nodes) {
      //   n->erase();
      // }
    }

    if (t > 0.1 && win.get_mouse_button(0)) {
      t = 0;
      tree->insert({win.mnorm, win.mnorm * 0.02f});
    }

    std::vector<TreeNode> reinsert;
    tree->draw(win.dt, reinsert);
    for (auto node : reinsert)
      if (!tree->rect.contains(node.pos))
        tree->insert(node);
  }
}