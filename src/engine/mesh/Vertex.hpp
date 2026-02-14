#pragma once

#include "VAO.hpp"

struct VertexP {
  vec3 position;

  static void link(const VAO& vao) {
    vao.linkAttrib(0, 3, GL_FLOAT, sizeof(VertexP), (void*)(0));
  }
};

struct VertexPCTN {
  vec3 position;
  vec3 color;
  vec2 texture;
  vec3 normal;

  static void link(const VAO& vao) {
    size_t typeSize = sizeof(float);
    GLsizei stride = sizeof(VertexPCTN);

    vao.linkAttrib(0, 3, GL_FLOAT, stride, (void*)(0 * typeSize));
    vao.linkAttrib(1, 3, GL_FLOAT, stride, (void*)(3 * typeSize));
    vao.linkAttrib(2, 2, GL_FLOAT, stride, (void*)(6 * typeSize));
    vao.linkAttrib(3, 3, GL_FLOAT, stride, (void*)(8 * typeSize));
  }
};

struct VertexPT {
  vec3 position;
  vec2 texture;

  static void link(const VAO& vao) {
    size_t typeSize = sizeof(float);
    GLsizei stride = sizeof(VertexPT);

    vao.linkAttrib(0, 3, GL_FLOAT, stride, (void*)(0 * typeSize));
    vao.linkAttrib(1, 2, GL_FLOAT, stride, (void*)(3 * typeSize));
  }
};

struct VertexPC {
  vec3 position;
  vec3 color;

  static void link(const VAO& vao) {
    size_t typeSize = sizeof(float);
    GLsizei stride = sizeof(VertexPC);

    vao.linkAttrib(0, 3, GL_FLOAT, stride, (void*)(0 * typeSize));
    vao.linkAttrib(1, 3, GL_FLOAT, stride, (void*)(3 * typeSize));
  }
};

