
#include <assert.h>
#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <type_traits>
#include <unordered_set>
#include <variant>
#include <vector>

#include "gfx.h"

#include "glad/glad.h"

using namespace std;

static Mesh mesh_cross, mesh_quad;
static Shader shader;

struct TreeNode {
  vec2 pos = {0, 0};
  vec2 vel = {0, 0};

  void update(f32 dt) {
    pos += vel * dt;
    const f32 v = len(vel);
    if (v > 0.01f)
      vel = norm(vel) * v * 0.999f;
  }

  void draw() {
    shader.set_uniform("pos", pos);
    shader.set_uniform("sz", vec2{0.0009f, 0.0016f});
    shader.set_uniform("col", vec4{0.8, 0.2, 0.7, 1});
    mesh_quad.draw();
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
  vec2 p = {0, 0};
  vec2 s = {0, 0};

  Range range() const { return {p - s * 0.5f, p + s * 0.5f}; }

  bool contains(Rect r) const { return range().contains(r.range()); }
  bool contains(vec2 v) const { return range().contains(Range{v, v}); }
  bool overlaps(Rect r) const { return range().overlaps(r.range()); }

  void divide(Rect r[4]) const {
    const vec2 hs = s * 0.5f;
    const vec2 qs = s * 0.25f;
    r[0] = {p + vec2{+qs.x, +qs.y}, hs};
    r[1] = {p + vec2{-qs.x, +qs.y}, hs};
    r[2] = {p + vec2{-qs.x, -qs.y}, hs};
    r[3] = {p + vec2{+qs.x, -qs.y}, hs};
  }
};

struct QuadTree {
  Rect rect;
  QuadTree* parent;

  variant<int, TreeNode, array<unique_ptr<QuadTree>, 4>> div;

  QuadTree(QuadTree* parent = 0, Rect rect = {{0, 0}, {2, 32.f / 9.f}})
      : parent(parent), rect(rect), div(0) {}

  TreeNode* node() { return get_if<TreeNode>(&div); }

  array<unique_ptr<QuadTree>, 4>* split() {
    return get_if<array<unique_ptr<QuadTree>, 4>>(&div);
  }

  void insert(TreeNode v) {
    if (!rect.contains(v.pos)) {
      // cout << rect.pos << rect.s << " Does not contain" << v.pos;
      return;
    }

    if (auto c = node()) {
      if (rect.s.x < 1.f / 512.f)
        return;

      Rect r[4];
      rect.divide(r);

      array<unique_ptr<QuadTree>, 4> tmp = {
          make_unique<QuadTree>(this, r[0]),
          make_unique<QuadTree>(this, r[1]),
          make_unique<QuadTree>(this, r[2]),
          make_unique<QuadTree>(this, r[3]),
      };

      tmp[get_quadrant(c->pos)]->insert(*c);
      tmp[get_quadrant(v.pos)]->insert(v);
      div = std::move(tmp);
      return;
    }

    if (auto c = split()) {
      (*c)[get_quadrant(v.pos)]->insert(v);
      return;
    }

    div = v;
  }

  int get_quadrant(vec2 v) {
    int x = v.x < rect.p.x;
    int y = v.y < rect.p.y;
    int q = (x ^ y) + 2 * y;
    if (!(q < 4 && q >= 0)) {
      assert(false);
    }
    return q;
  }

  bool is_none() { return holds_alternative<int>(div); }

  bool has_children() {
    bool re = false;
    if (auto c = split())
      for (auto& c : *c)
        re |= !c->is_none();
    return re;
  }

  void erase_down() {
    if (node())
      return;

    if (auto c = split()) {
      if (!has_children())
        div = 0;
      else {
        for (auto& c : *c)
          c->erase_down();
        if (!has_children())
          div = 0;
      }
      return;
    }
  }

  void erase_up() {
    if (node())
      return;

    if (auto c = split())
      if (!has_children())
        div = 0;

    if (parent) {
      parent->erase_up();
    }
  }

  void erase() {
    div = 0;
    if (parent)
      parent->erase_up();
  }
  void update(f32 dt, vector<TreeNode>& v) {
    if (auto c = split()) {
      for (auto& c : *c)
        c->update(dt, v);
    }

    if (auto c = node()) {
      c->update(dt);
      if (!rect.contains(c->pos)) {
        v.push_back(*c);
        div = 0;
      }
    }
  }

  void draw() {
    if (auto c = split()) {
      shader.set_uniform("pos", rect.p);
      shader.set_uniform("sz", rect.s * 0.5f);
      shader.set_uniform("col", vec4{1, 1, 1, 1});
      mesh_cross.draw();
      for (auto& c : *c)
        c->draw();
      return;
    }

    if (auto c = node())
      c->draw();
  }

  void collect(vector<QuadTree*>& collection) {
    if (auto c = node()) {
      collection.push_back(this);
      return;
    }

    if (auto c = split())
      for (auto& c : *c)
        c->collect(collection);
  }

  void find(Rect r, vector<QuadTree*>& collection) {
    if (is_none())
      return;

    if (r.contains(rect)) {
      collect(collection);
      return;
    }

    if (!r.overlaps(rect)) {
      return;
    }

    if (auto c = node()) {
      if (r.contains(c->pos))
        collection.push_back(this);
      return;
    }

    for (auto& c : *split())
      c->find(r, collection);
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

  {
    vec2 v[4] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    vec2 u[4] = {{-1, -1}, {-1, 1}, {1, 1}, {1, -1}};
    Mesh::AttribLayout layout = {0, 2, 8, 0};
    mesh_cross.create(v, sizeof v, 4, &layout, 1, Primitive::Lines);
    mesh_quad.create(u, sizeof u, 4, &layout, 1, Primitive::Quads);
  }

  while (win.poll()) {
    shader.bind();

    if (win.get_mouse_button(1)) {
      const f32 sz = 1.f / 16.f;
      const f32 ar = 16.f / 9.f;
      shader.set_uniform("pos", win.mnorm);
      shader.set_uniform("sz", vec2{1, ar} * sz * 0.5f);
      shader.set_uniform("col", vec4{0.2, 0.6, 1.f, 0.4});
      mesh_quad.draw();

      vector<QuadTree*> v;
      tree->find(Rect{win.mnorm, vec2{1, ar} * sz}, v);

      for (auto& q : v) {
        q->erase();
      }
    }

    if (win.get_mouse_button(0)) {
      tree->insert({win.mnorm, win.mdelta * 0.125f});
    }

    tree->draw();

    vector<TreeNode> rv;

    tree->update(win.dt, rv);
    for (auto& c : rv)
      tree->insert(c);

    tree->erase_down();
  }
}