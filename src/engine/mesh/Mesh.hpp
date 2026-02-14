#pragma once

#include <functional>
#include <span>
#include <vector>

#include "Vertex.hpp"
#include "Transformable.hpp"
#include "VAO.hpp"
#include "BufferObject.hpp"
#include "../Camera.hpp"
#include "../Shader.hpp"

class Mesh : public Transformable {
public:
  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  Mesh(Mesh&& other);
  Mesh& operator=(Mesh&& other);

  Mesh(const std::vector<VertexPCTN>& vertices, const std::vector<GLuint>& indices, GLenum mode, GLenum usage = GL_STATIC_DRAW);
  Mesh(const std::vector<VertexPT>&   vertices, const std::vector<GLuint>& indices, GLenum mode, GLenum usage = GL_STATIC_DRAW);
  Mesh(const std::vector<VertexPC>&   vertices, const std::vector<GLuint>& indices, GLenum mode, GLenum usage = GL_STATIC_DRAW);

  Mesh(const std::vector<VertexPCTN>& vertices, GLenum mode, GLenum usage);
  Mesh(const std::vector<VertexPT>&   vertices, GLenum mode, GLenum usage);
  Mesh(const std::vector<VertexPC>&   vertices, GLenum mode, GLenum usage);

  ~Mesh();

  static Mesh loadObj(const fspath& file, bool printInfo = false);
  static void screenDraw(const Camera* camera, Shader& shader);

  template<typename V>
  void updateData(const std::vector<V>& vertices, GLenum usage) {
    vbo.allocate(vertices, usage);
    count = vertices.size();
  }

  void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;
  void clear();

private:
  friend struct gui;

  size_t verticesCount = 0;
  size_t indicesCount = 0;

  GLsizei count = 0;
  GLenum mode = 0;
  VAO vao;
  BufferObject vbo{GL_ARRAY_BUFFER};
  BufferObject ebo{GL_ELEMENT_ARRAY_BUFFER};

  std::function<void(GLenum mode, GLsizei count)> drawFunc;

private:
  static void setCamUniforms(const Camera* c, Shader& s);

  static void drawElements(GLenum mode, GLsizei count);
  static void drawArrays(GLenum mode, GLsizei count);

  template<typename V>
  Mesh(const std::span<const V> v, const std::span<const GLuint> i, GLenum mode, GLenum usage)
    : verticesCount(v.size()),
      indicesCount(i.size()),
      count(indicesCount ?: verticesCount),
      mode(mode),
      drawFunc(indicesCount ? drawElements : drawArrays)
  {
    bool useEBO = indicesCount;

    vao.gen();
    vbo.allocate(v, usage);
    if (useEBO) ebo.allocate(i, usage);

    vao.bind();
    vbo.bind();
    if (useEBO) ebo.bind();

    V::link(vao);

    vao.unbind();
    vbo.unbind();
    if (useEBO) ebo.unbind();
  }
};

