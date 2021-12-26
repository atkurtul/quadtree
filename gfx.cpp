
#include "glad/glad.h"

#include <GLFW/glfw3.h>
#include <corecrt_math.h>

#include "gfx.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>

void gl_set_viewport(ivec2 pos, ivec2 size) {
  glViewport(pos.x, pos.y, size.x, size.y);
}

void gl_set_state(State state, bool set) {
  set ? glEnable((u32)state) : glDisable((u32)state);
}

void gl_draw_arrays(Primitive primitive, u32 vertices) {
  glDrawArrays((u32)primitive, 0, vertices);
}

void gl_set_wireframe(bool c) {
  glPolygonMode(GL_FRONT, c ? GL_LINE : GL_FILL);
  glPolygonMode(GL_BACK, c ? GL_LINE : GL_FILL);
}

void GLAPIENTRY gl_message_callback(GLenum source,
                                    GLenum type,
                                    GLuint id,
                                    GLenum severity,
                                    GLsizei length,
                                    const GLchar* message,
                                    const void* userParam) {
  const char* sourcestr = "";

  switch (source) {
    case GL_DEBUG_SOURCE_API:
      sourcestr = "API";
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      sourcestr = "Window System";
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      sourcestr = "Shader Compiler";
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      sourcestr = "Third Party";
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      sourcestr = "Application";
      break;
    case GL_DEBUG_SOURCE_OTHER:
      sourcestr = "Other";
      break;
  }

  const char* severitystr = "";
  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      severitystr = "High";
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      severitystr = "Medium";
      break;
    case GL_DEBUG_SEVERITY_LOW:
      severitystr = "Low";
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      return;
      break;
  }

  const char* errorstr = "";
  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      errorstr = "Error";
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      errorstr = "Deprecated behavior";
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      errorstr = "Undefined behavior";
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      errorstr = "Portability";
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      errorstr = "Performance";
      break;
    case GL_DEBUG_TYPE_MARKER:
      errorstr = "Marker";
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      errorstr = "Push group";
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      errorstr = "Pop group";
      break;
    case GL_DEBUG_TYPE_OTHER:
      errorstr = "Other";
    default:
      errorstr = "Other";
      break;
  }

  printf("[OpenGL] [%s] [%s] [%s severity] %s", errorstr, sourcestr,
         severitystr, message);
}

void Mesh::update_buffer(void* dat,
                         u64 size,
                         u64 nverts,
                         const AttribLayout* layout,
                         size_t nlayout) {
  this->nverts = nverts;
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, size, dat, GL_STATIC_DRAW);

  while (--nlayout) {
    glEnableVertexAttribArray(layout->idx);
    glVertexAttribPointer(layout->idx, layout->size, GL_FLOAT, 0,
                          layout->stride, (void*)(u64)layout->offset);
    ++layout;
  }
}

void Mesh::create(void* dat,
                  u64 size,
                  u64 nverts,
                  const AttribLayout* layout,
                  size_t nlayout,
                  Primitive primitive) {
  this->nverts = nverts;
  this->primitive = primitive;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glGenBuffers(1, &vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, size, dat, GL_STATIC_DRAW);

  while (nlayout--) {
    glEnableVertexAttribArray(layout->idx);
    glVertexAttribPointer(layout->idx, layout->size, GL_FLOAT, 0,
                          layout->stride, (void*)(u64)layout->offset);
    ++layout;
  }
}

void Mesh::free() {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
}

void Mesh::draw() {
  if (nverts) {
    glBindVertexArray(vao);
    glDrawArrays((u32)primitive, 0, nverts);
  }
}

void Texture::bind(u32 slot) const {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(kind, id);
}

void Texture::load_from_file(const char* file, int face) {
  int w, h, d;

  u8* data = stbi_load(file, &w, &h, &d, 4);
  if (!data) {
    printf("Could not load texture %s\n", file);
    return;
  }
  printf("Successfully loaded texture %s\n", file);

  size = ivec2{w, h};

  update(data, w, h, Format::RGBA, face);
  ::free(data);
}

void Texture::load_from_memory(u8* buf, u64 len) {
  int w, h, d;

  u8* data = stbi_load_from_memory(buf, len, &w, &h, &d, 4);
  if (!data) {
    printf("Could not load texture from memory.");
    return;
  }
  printf("Successfully loaded texture from memory.");

  size = ivec2{w, h};

  update(data, w, h, Format::RGBA);
  ::free(data);
}

void Texture::generate(Kind kind) {
  this->kind = kind;
  GL_TEXTURE_2D;
  glGenTextures(1, &id);
  glBindTexture(kind, id);
  glTexParameteri(kind, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(kind, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(kind, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(kind, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(kind, 0);
}

void Texture::update(const void* data, i32 w, i32 h, Format format, int face) {
  size = ivec2{w, h};
  glBindTexture(kind, id);
  glTexImage2D(kind, 0, GL_RGBA, w, h, 0, (u32)format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(kind);
  glBindTexture(kind, 0);
}

void Texture::free() {
  glDeleteTextures(1, &id);
}

f32 Texture::aspect_ratio() const {
  return (f32)size.x / (f32)size.y;
}

u32 compile_shader(const char* src, u32 type) {
  u32 id = glCreateShader(type);
  glShaderSource(id, 1, &src, NULL);
  glCompileShader(id);
  int len = 0, res = 1;
  glGetShaderiv(id, GL_COMPILE_STATUS, &res);
  glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
  if (!res) {
    static char buf[4096];
    glGetShaderInfoLog(id, len, 0, buf);
    printf("Failed to compile shader :\n%.*s.", len, buf);
  }
  return id;
}

void Shader::load(const char* vsrc, const char* psrc) {
  id = glCreateProgram();

  glAttachShader(id, compile_shader(vsrc, GL_VERTEX_SHADER));
  glAttachShader(id, compile_shader(psrc, GL_FRAGMENT_SHADER));
  glLinkProgram(id);

  int len = 0, res = 1;
  glGetProgramiv(id, GL_COMPILE_STATUS, &res);
  glGetProgramiv(id, GL_INFO_LOG_LENGTH, &len);

  if (!res) {
    static char buf[4096];
    glGetProgramInfoLog(id, len, 0, buf);
    printf("Failed to generate shader:\n%.*s.", len, buf);
    return;
  }
  glUseProgram(id);
}

void Shader::free() {
  glDeleteProgram(id);
}

void Shader::bind() {
  glUseProgram(id);
}

void Shader::set_uniform(const char* name, bool data) {
  glUniform1i(glGetUniformLocation(id, name), data);
}

void Shader::set_uniform(const char* name, float data) {
  glUniform1f(glGetUniformLocation(id, name), data);
}

void Shader::set_uniform(const char* name, mat4 data) {
  glUniformMatrix4fv(glGetUniformLocation(id, name), 1, 0, &data[0][0]);
}

void Shader::set_uniform(const char* name, mat2 data) {
  glUniformMatrix2fv(glGetUniformLocation(id, name), 1, 0, &data[0][0]);
}

void Shader::set_uniform_array(const char* name, const mat4* data, u32 size) {
  glUniformMatrix4fv(glGetUniformLocation(id, name), size, 0, (f32*)data);
}

void Shader::set_uniform_array(const char* name, const vec2* data, u32 size) {
  glUniform2fv(glGetUniformLocation(id, name), size, (f32*)data);
}

void Shader::set_uniform(const char* name, vec2 data) {
  glUniform2fv(glGetUniformLocation(id, name), 1, &data[0]);
}

void Shader::set_uniform(const char* name, vec3 data) {
  glUniform3fv(glGetUniformLocation(id, name), 1, &data[0]);
}

void Shader::set_uniform(const char* name, vec4 data) {
  glUniform4fv(glGetUniformLocation(id, name), 1, &data[0]);
}

static void wheel_callback(GLFWwindow* w, f64 x, f64 y) {
  auto win = (Window*)glfwGetWindowUserPointer(w);
  win->wheel = y;
}

Window::Window(const char* name) {
  if (!glfwInit()) {
    printf("Failed to create window\n");
    exit(-1);
  } else {
    printf("Window successfully created\n");
  }

  glfwWindowHint(GLFW_SAMPLES, 4);

  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);

  glfwWindowHint(GLFW_RED_BITS, mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
  // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

  printf("Monitor size: %dx%d\n", mode->width, mode->height);

  win = glfwCreateWindow(mode->width, mode->height, name, 0, 0);
  glfwMakeContextCurrent(win);

  glfwSetInputMode(win, GLFW_RAW_MOUSE_MOTION, glfwRawMouseMotionSupported());
  // glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGL()) {
    printf("Failed to init GL\n");
    exit(-1);
  }
  printf("OpenGL version %s\n", glGetString(GL_VERSION));

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_MULTISAMPLE);
  glClearColor(0, 0, 0, 1);
  glEnable(GL_DEPTH_TEST);
  // glEnable(GL_CULL_FACE);
  // glFrontFace(GL_CW);
  glfwSwapInterval(0);
  glDebugMessageCallback(gl_message_callback, 0);
  glLineWidth(1.f);

  glfwSetWindowUserPointer(win, this);
  glfwSetScrollCallback(win, wheel_callback);
}

Window::~Window() {
  glfwDestroyWindow(win);
}

bool Window::poll() {
  wheel = 0;
  glfwPollEvents();
  glfwSwapBuffers(win);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  double x, y;
  glfwGetCursorPos(win, &x, &y);
  vec2 tmp = {(f32)x, (f32)y};
  mdelta = tmp - mpos;
  mpos = tmp;
  ivec2 size = get_size();
  sz.x = size.x;
  sz.y = size.y;

  mnorm = 2.f * (mpos / sz) - vec2{1.f, 1.f};
  mnorm.y *= -1;

  {
    float tmp = glfwGetTime();
    dt = tmp - time;
    time = tmp;
  }
  
  if (get_key(Key::LeftControl)) {
    if (get_key(Key::_0))
      switch_to_monitor(0);
    if (get_key(Key::_1))
      switch_to_monitor(1);
  }

  if (get_key(Key::Escape)) {
    glfwSetWindowShouldClose(win, 1);
  }
  return !glfwWindowShouldClose(win);
}

void Window::close() {
  glfwSetWindowShouldClose(win, 1);
}

bool Window::get_key(Key keycode) const {
  return glfwGetKey(win, (int)keycode);
}

bool Window::get_mouse_button(int keycode) const {
  return glfwGetMouseButton(win, keycode);
}

void Window::toggle_mouse(int state) {
  glfwSetInputMode(win, GLFW_CURSOR, state);
}

void Window::set_size(uvec2 s) {
  glfwSetWindowSize(win, s.x, s.y);
}

ivec2 Window::get_size() {
  ivec2 s;
  glfwGetWindowSize(win, &s.x, &s.y);
  return s;
}

void Window::switch_to_monitor(u32 idx) {
  if (monitor == idx) {
    return;
  }

  int count;
  auto monitors = glfwGetMonitors(&count);

  if (idx >= count) {
    printf("Failed to switch to monitor %d.", idx);
    return;
  }

  monitor = idx;
  const GLFWvidmode* mode = glfwGetVideoMode(monitors[idx]);
  glfwSetWindowMonitor(win, monitors[idx], 0, 0, mode->width, mode->height, 0);
  glfwFocusWindow(win);
  printf("Switched to monitor %d. Size %dx%d.", idx, mode->width, mode->height);
}
