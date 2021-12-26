R"(#version 450

uniform vec3 col;
out vec4 out_color;

void main() {
  out_color = vec4(col,1);
})"
