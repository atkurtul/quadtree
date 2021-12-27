R"(#version 330

in vec2 vert;

uniform vec2 sz;
uniform vec2 pos;

uniform vec2 cam_pos;
uniform float zoom;

uniform mat2 mrot;

void main() {
  gl_Position = vec4((pos + sz * vert - cam_pos) / zoom, 0, 1);
  gl_Position.xy = mrot * gl_Position.xy;
})"