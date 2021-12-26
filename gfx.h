#pragma once

#include "vec.h"

#define M_PI 3.14159265358979323846f  // pi
#define H_PI 1.57079632679f
#define F_PI 6.28318530718f

enum class Primitive;
enum class State;
enum class InputEventType;
enum class Key;
enum class MouseButton;
enum class Format;
enum class Usage;

void gl_set_viewport(ivec2 pos, ivec2 size);
void gl_set_state(State state, bool set);
void gl_draw_arrays(Primitive primitive, u32 vertices);
void gl_set_wireframe(bool);


enum class Primitive {
  Points = 0x0000,         // GL_POINT,
  Lines = 0x0001,          // GL_LINE,
  LineLoop = 0x0002,       // GL_LINE_LOO,
  LineStrip = 0x0003,      // GL_LINE_STRI,
  Triangles = 0x0004,      // GL_TRIANGLE,
  TriangleStrip = 0x0005,  // GL_TRIANGLE_STRI,
  TriangleFan = 0x0006,    // GL_TRIANGLE_FA,
  Quads = 0x0007,          // GL_QUAD,
};

enum class State {
  DepthTest = 0x0B71,    // GL_DEPTH_TEST,
  Multisample = 0x809D,  // GL_MULTISAMPLE,
  DebugOutput = 0x92E0,  // GL_DEBUG_OUTPUT,
};

enum class Usage {
  StaticDraw = 0x88E4,
  DynamicDraw = 0x88E8,
};

enum class Format {
  Red = 0x1903,
  Green = 0x1904,
  Blue = 0x1905,
  Alpha = 0x1906,
  RGB = 0x1907,
  RGBA = 0x1908,
  BGR = 0x80E0,
  BGRA = 0x80E1,
};

struct Mesh {
  u32 nverts = 0;
  u32 vao;
  u32 vbo;
  Primitive primitive;

  struct AttribLayout {
    u32 idx;
    u32 size;
    u32 stride;
    u32 offset;
  };

  void update_buffer(void* dat,
                     u64 size,
                     u64 nverts,
                     const AttribLayout* layout,
                     size_t nlayout);
  void create(void* dat,
              u64 size,
              u64 nverts,
              const AttribLayout* layout,
              size_t nlayout,
              Primitive);

  void free();
  void draw();
};


struct Texture {
  enum Kind {
    _2D = 0x0DE1,
    CubeMap = 0x8513,
  } kind = _2D;

  u32 id;
  ivec2 size = {};
  void bind(unsigned) const;
  void generate(Kind = _2D);
  void update(const void* data, i32 w, i32 h, Format format, int face = 0);
  void free();
  void load_from_file(const char* file, int face = 0);
  void load_from_memory(u8* buf, u64 len);
  f32 aspect_ratio() const;
};

struct Shader {
  u32 id;
  void load(const char* vsrc, const char* psrc);
  void free();
  void bind();
  void set_uniform(const char* name, bool data);
  void set_uniform(const char* name, float data);
  void set_uniform_array(const char* name, const mat4* data, u32 size);
  void set_uniform_array(const char* name, const vec2* data, u32 size);
  void set_uniform(const char* name, mat4 data);
  void set_uniform(const char* name, mat3 data);
  void set_uniform(const char* name, mat2 data);
  void set_uniform(const char* name, vec2 data);
  void set_uniform(const char* name, vec3 data);
  void set_uniform(const char* name, vec4 data);
};

struct Window {
  struct GLFWwindow* win = 0;
  vec2 mdelta = {};
  vec2 mpos = {};
  vec2 mnorm = {};
  vec2 sz = {};
  int wheel = 0;
  f32 time = 0;
  f32 dt = 0;
  int monitor = 0;

  Window(const char* name);
  ~Window();
  bool poll();
  bool get_key(Key) const;
  bool get_mouse_button(int keycode) const;
  void toggle_mouse(int state);
  void set_size(uvec2);
  ivec2 get_size();
  void close();
  void switch_to_monitor(u32);
};

enum class Key {
  Space = 32,
  Apostrophe = 39 /* ' */,
  Comma = 44 /* , */,
  Minus = 45 /* - */,
  Period = 46 /* . */,
  Slash = 47 /* / */,
  _0 = 48,
  _1 = 49,
  _2 = 50,
  _3 = 51,
  _4 = 52,
  _5 = 53,
  _6 = 54,
  _7 = 55,
  _8 = 56,
  _9 = 57,
  Semicolon = 59 /* ; */,
  Equal = 61 /* = */,
  A = 65,
  B = 66,
  C = 67,
  D = 68,
  E = 69,
  F = 70,
  G = 71,
  H = 72,
  I = 73,
  J = 74,
  K = 75,
  L = 76,
  M = 77,
  N = 78,
  O = 79,
  P = 80,
  Q = 81,
  R = 82,
  S = 83,
  T = 84,
  U = 85,
  V = 86,
  W = 87,
  X = 88,
  Y = 89,
  Z = 90,
  LeftBracket = 91 /* [ */,
  Backslash = 92 /* \ */,
  RightBracket = 93 /* ] */,
  GraveAccent = 96 /* ` */,
  World1 = 161 /* non-US #1 */,
  World2 = 162 /* non-US #2 */,
  Escape = 256,
  Enter = 257,
  Tab = 258,
  Backspace = 259,
  Insert = 260,
  Delete = 261,
  Right = 262,
  Left = 263,
  Down = 264,
  Up = 265,
  PageUp = 266,
  PageDown = 267,
  Home = 268,
  End = 269,
  CapsLock = 280,
  ScrollLock = 281,
  NumLock = 282,
  PrintScreen = 283,
  Pause = 284,
  F1 = 290,
  F2 = 291,
  F3 = 292,
  F4 = 293,
  F5 = 294,
  F6 = 295,
  F7 = 296,
  F8 = 297,
  F9 = 298,
  F10 = 299,
  F11 = 300,
  F12 = 301,
  F13 = 302,
  F14 = 303,
  F15 = 304,
  F16 = 305,
  F17 = 306,
  F18 = 307,
  F19 = 308,
  F20 = 309,
  F21 = 310,
  F22 = 311,
  F23 = 312,
  F24 = 313,
  F25 = 314,
  KP0 = 320,
  KP1 = 321,
  KP2 = 322,
  KP3 = 323,
  KP4 = 324,
  KP5 = 325,
  KP6 = 326,
  KP7 = 327,
  KP8 = 328,
  KP9 = 329,
  KPDECIMAL = 330,
  KPDivide = 331,
  KPMultiply = 332,
  KPSubtract = 333,
  KPAdd = 334,
  KPEnter = 335,
  KPEqual = 336,
  LeftShift = 340,
  LeftControl = 341,
  LeftAlt = 342,
  LeftSuper = 343,
  RightShift = 344,
  RightControl = 345,
  RightAlt = 346,
  RightSuper = 347,
  Menu = 348,
};
