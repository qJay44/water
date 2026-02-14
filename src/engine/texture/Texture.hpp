#pragma once

#include "TextureDescriptor.hpp"
#include "image2D.hpp"
#include <future>

class Texture {
public:
  static const Texture& getDebug0Tex();

  Texture() = default;

  Texture(Texture&& other);

  Texture& operator=(Texture&& other);

  Texture(const image2D& img, const TextureDescriptor& desc);
  Texture(const ivec2& size, const TextureDescriptor& desc);
  Texture(const fspath& path, const TextureDescriptor& desc);
  ~Texture();

  void update();
  void upload(ivec2 size, void* pixels);
  void bind(GLuint customUnit) const;
  void bind() const;
  void unbind() const;
  void clear();

  const GLuint& getId() const;
  const GLenum& getTarget() const;
  const GLuint& getUnit() const;
  const std::string& getUniformName() const;
  // const uvec3& getSize() const;

  void setUnit(GLuint unit);
  void setUniformName(const std::string& name);

protected:
  static Texture debug0Tex;

  TextureDescriptor desc{};
  GLuint id = 0;

  struct AsyncData {
    image2D images[6];
  };

  std::future<AsyncData> texFuture;
  bool loaded = false;

protected:
  static AsyncData loadCubemap(fspath folder, GLenum internalFormat);

  void create2D(const image2D& img);
  void create2DArray(const fspath& folder);
  void createCubemap(const AsyncData& data);
};

