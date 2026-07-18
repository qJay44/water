#include "Texture2D.hpp"

Texture2D Texture2D::debug0Tex;

const Texture2D& Texture2D::getDebugTex0() {
  if (debug0Tex.id == 0)
    debug0Tex = Texture2D(
      {"res/tex/debug/uvChecker.jpg", IMAGE2D_LOAD_STB , false},
      {
        .minFilter = GL_NEAREST,
        .magFilter = GL_NEAREST,
        .wrapS = GL_REPEAT,
        .wrapT = GL_REPEAT,
    });

  return debug0Tex;
}

void Texture2D::initStorage(const image2D& img, const TextureDescriptor& desc) {
  onInit(desc);
  glTexStorage2D(target, 1, desc.internalFormat, img.width, img.height);

  if (img.pixels)
    glTexSubImage2D(target, 0, 0, 0, img.width, img.height, desc.format, desc.type, img.pixels);

  unbind();
}

void Texture2D::initImage(const image2D& img, const TextureDescriptor& desc) {
  onInit(desc);
  glTexImage2D(target, 0, desc.internalFormat, img.width, img.height, 0, desc.format, desc.type, img.pixels);
  unbind();
}

Texture2D::Texture2D(const image2D& img, const TextureDescriptor& desc) {
  initStorage(img, desc);
}

Texture2D::Texture2D(const ivec2& size, const TextureDescriptor& desc)
  : Texture2D(image2D{size.x, size.y}, desc) {}

Texture2D::Texture2D(int size, const TextureDescriptor& desc)
  : Texture2D(image2D{size, size}, desc) {}

Texture2D::Texture2D(const fspath& path, const TextureDescriptor& desc)
  : Texture2D(image2D(path), desc) {}

void Texture2D::upload(ivec2 coord, ivec2 size, const void* data, GLenum format, GLenum type) const {
  bind(0);
  glTexSubImage2D(target, 0, coord.x, coord.y, size.x, size.y, format, type, data);
  unbind();
}

void Texture2D::onInit(const TextureDescriptor& desc) {
  if (desc.target != GL_TEXTURE_2D)
    error("[Texture2D::Texture2D] Wrong target ({:#x})", desc.target);

  target = desc.target;

  clear();
  glGenTextures(1, &id);
  bind();
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, desc.minFilter);
  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, desc.magFilter);
  glTexParameteri(target, GL_TEXTURE_WRAP_S, desc.wrapS);
  glTexParameteri(target, GL_TEXTURE_WRAP_T, desc.wrapT);
}

