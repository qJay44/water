#pragma once

#include "texture/Texture.hpp"

struct FBO {
public:
  FBO() {
    gen();
  }

  static void unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void gen() {
    glGenFramebuffers(size, &id);
  }

  void bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
  }

  void attach2D(GLenum attachment, const Texture& tex) const {
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, tex.getTarget(), tex.getId(), 0);
    unbind();
  }

  void clear() {
    if (id) glDeleteFramebuffers(size, &id);
    id = 0;
  }

private:
  static constexpr GLsizei size = 1;
  GLuint id = 0;
};

