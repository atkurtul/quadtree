R"(#version 450

in  vec2 vert;

uniform vec2 sz;
uniform vec2 pos;

void main() {
  gl_Position = vec4(pos + sz * vert, -1, 1);
})"