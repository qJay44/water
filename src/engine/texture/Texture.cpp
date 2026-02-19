#include "Texture.hpp"
#include "utils/utils.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <future>
#include <string>
#include <utility>
#include <vector>

Texture Texture::debug0Tex;

const Texture& Texture::getDebug0Tex() {
  if (debug0Tex.desc.uniformName.empty())
    debug0Tex = Texture("res/tex/debug/uvChecker.jpg", {
      .uniformName = "u_debug0Tex",
      .unit = 0, // NOTE: Can be changed
      .minFilter = GL_NEAREST,
      .magFilter = GL_NEAREST,
      .genMipMap = false
    });

  return debug0Tex;
}

Texture::Texture(Texture&& other) :
  desc(other.desc),
  id(other.id),
  texFuture(std::move(other.texFuture))
  // loaded(other.loaded)
{
  other.id = 0;
  other.loaded = false;
}

Texture& Texture::operator=(Texture&& other) {
  if (this != &other) {
    clear();
    desc = other.desc;
    id = other.id;
    texFuture = std::move(other.texFuture);
    loaded = other.loaded;
    other.id = 0;
    // other.loaded = false;
  }

  return *this;
}

Texture::Texture(const image2D& img, const TextureDescriptor& d) : desc(d) {
  if (desc.target != GL_TEXTURE_2D)
    error("[Texture::Texture] Unhandled texture creation type: [{}]", desc.target);

  create2D(img);
}

Texture::Texture(const ivec2& size, const TextureDescriptor& desc) : Texture(image2D{size.x, size.y}, desc) {}

Texture::Texture(const fspath& path, const TextureDescriptor& d) : desc(d) {
  loaded = true;

  switch (desc.target) {
    case GL_TEXTURE_2D:
      create2D(image2D(path));
      break;
    case GL_TEXTURE_2D_ARRAY:
      create2DArray(path);
      break;
    case GL_TEXTURE_CUBE_MAP:
      if (desc.flags & TextureFlags_AsyncLoad) {
        loaded = false;
        std::function<void(fspath, GLenum)> func;
        switch (desc.cubemapLoad) {
          case TextureCubemapLoad_FromFolder:
            texFuture = std::async(std::launch::async, loadCubemapFromFolderAsync, path, desc.internalFormat);
            break;
          case TextureCubemapLoad_FromPanoramaImage:
          case TextureCubemapLoad_FromCubemapImage:
            error("[Texture::Texture] Not implemented");
            break;
        }
      } else {
        switch (desc.cubemapLoad) {
          case TextureCubemapLoad_FromFolder:
          case TextureCubemapLoad_FromPanoramaImage:
            error("[Texture::Texture] Not implemented");
            break;
          case TextureCubemapLoad_FromCubemapImage:
            createCubemapFromCubemapImage(image2D(path));
            break;
        }
      }
      break;
    default:
      error("[Texture::Texture] Unhandled texture creation type: [{}]", desc.target);
      break;
  }
}

Texture::~Texture() {
  clear();
}

void Texture::update() {
  if (!loaded && texFuture.valid()) {
    auto status = texFuture.wait_for(std::chrono::milliseconds(0));
    if (status == std::future_status::ready) {
      switch (desc.target) {
        case GL_TEXTURE_CUBE_MAP:
          createCubemapFromFolderAsync(texFuture.get());
          break;
        default:
          error("[Texture::update] Should never happen [{}]", desc.target);
          break;
      }
      loaded = true;
    }
  }
}

void Texture::upload(ivec2 size, void* pixels) {
  assert(desc.target == GL_TEXTURE_2D);
  glTextureSubImage2D(id, 0, 0, 0, size.x, size.y, desc.format, desc.type, pixels);
}

void Texture::bind(GLuint customUnit) const {
  glActiveTexture(GL_TEXTURE0 + customUnit);
  glBindTexture(desc.target, id);
}

void Texture::bind() const {
  bind(desc.unit);
}

void Texture::unbind() const {
  glBindTexture(desc.target, 0);
}

void Texture::clear() {
  if (id) glDeleteTextures(1, &id);
  id = 0;
}

const TextureDescriptor& Texture::getDescriptor() const { return desc; }
const GLuint& Texture::getId() const { return id; }
const GLenum& Texture::getTarget() const { return desc.target; }
const GLuint& Texture::getUnit() const { return desc.unit; }
const std::string& Texture::getUniformName() const { return desc.uniformName; }
// const ivec2& Texture::getSize() const { return desc.size; }

void Texture::setUnit(GLuint unit) {
  this->desc.unit = unit;
}

void Texture::setUniformName(const std::string& name) {
  desc.uniformName = name;
}

void Texture::create2D(const image2D& img) {
  assert(desc.target == GL_TEXTURE_2D);

  glGenTextures(1, &id);
  bind();
  glTexParameteri(desc.target, GL_TEXTURE_MIN_FILTER, desc.minFilter);
  glTexParameteri(desc.target, GL_TEXTURE_MAG_FILTER, desc.magFilter);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_S, desc.wrapS);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_T, desc.wrapT);
  glTexImage2D(desc.target, 0, desc.internalFormat, img.width, img.height, 0, desc.format, desc.type, img.pixels);

  if (desc.genMipMap)
    glGenerateMipmap(desc.target);

  unbind();
}

void Texture::create2DArray(const fspath& folder) {
  assert(desc.target == GL_TEXTURE_2D_ARRAY);

  std::vector<fspath> paths;
  for (const auto& entry : std::filesystem::directory_iterator(folder))
    paths.push_back(entry.path());

  std::ranges::sort(paths);

  image2D img0(paths[0], desc.internalFormat);

  glGenTextures(1, &id);
  bind();
  glTexParameteri(desc.target, GL_TEXTURE_MIN_FILTER, desc.minFilter);
  glTexParameteri(desc.target, GL_TEXTURE_MAG_FILTER, desc.magFilter);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_S, desc.wrapS);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_T, desc.wrapT);
  glTexStorage3D(desc.target, 1, desc.internalFormat, img0.width, img0.height, paths.size());
  glTexSubImage3D(desc.target, 0, 0, 0, 0, img0.width, img0.height, 1, desc.format, desc.type, img0.pixels);

  for (size_t i = 1; i < paths.size(); i++) {
    image2D img(paths[i], desc.internalFormat);
    assert(img0.width == img.width);
    assert(img0.height == img.height);
    assert(img0.channels == img.channels);

    glTexSubImage3D(desc.target, 0, 0, 0, i, img.width, img.height, 1, desc.format, desc.type, img.pixels);
  }

  if (desc.genMipMap)
    glGenerateMipmap(desc.target);

  unbind();
}

Texture::AsyncData Texture::loadCubemapFromFolderAsync(fspath folder, GLenum internalFormat) {
  namespace fs = std::filesystem;

  // NOTE: Order matters
  constexpr const char* texNames[6] = {
    "right",
    "left",
    "top",
    "bottom",
    "front",
    "back",
  };

  std::string extension;
  if (fs::exists(folder) && fs::is_directory(folder)) {
    auto it = fs::directory_iterator(folder);
    if (it != std::filesystem::end(it))
      extension = it->path().extension().string();
    else
      error("[Texture] Folder is empty [{}]", folder.string());
  } else {
    error("[Texture] Incorrect path [{}]", folder.string());
  }

  AsyncData data;

  for (int i = 0; i < 6; i++) {
    fspath filePath = std::format("{}/{}{}", folder.string().c_str(), texNames[i], extension.c_str());
    data.images[i] = image2D(filePath, internalFormat);
  }

  return data;
}

void Texture::createCubemapFromFolderAsync(const AsyncData& data) {
  glGenTextures(1, &id);
  bind();
  glTexParameteri(desc.target, GL_TEXTURE_MIN_FILTER, desc.minFilter);
  glTexParameteri(desc.target, GL_TEXTURE_MAG_FILTER, desc.magFilter);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_S, desc.wrapS);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_T, desc.wrapT);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_R, desc.wrapR);

  for (int i = 0; i < 6; i++) {
    const image2D& img = data.images[i];
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, desc.internalFormat, img.width, img.height, 0, desc.format, desc.type, img.pixels);
  }

  if (desc.genMipMap)
    glGenerateMipmap(desc.target);
}

void Texture::createCubemapFromCubemapImage(const image2D& img) {
  glGenTextures(1, &id);
  bind();
  glTexParameteri(desc.target, GL_TEXTURE_MIN_FILTER, desc.minFilter);
  glTexParameteri(desc.target, GL_TEXTURE_MAG_FILTER, desc.magFilter);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_S, desc.wrapS);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_T, desc.wrapT);
  glTexParameteri(desc.target, GL_TEXTURE_WRAP_R, desc.wrapR);

  //     +Y
  //  -X +Z +X -Z
  //     -Y
  constexpr int skipCols[6] = {2, 0, 1, 1, 1, 3};
  constexpr int skipRows[6] = {1, 1, 0, 2, 1, 1};

  int faceSize = img.width / 4;
  glPixelStorei(GL_UNPACK_ROW_LENGTH, img.width);

  for (int i = 0; i < 6; i++) {
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, faceSize * skipCols[i]);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, faceSize * skipRows[i]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, desc.internalFormat, faceSize, faceSize, 0, desc.format, desc.type, img.pixels);
  }

  if (desc.genMipMap)
    glGenerateMipmap(desc.target);

  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
}

