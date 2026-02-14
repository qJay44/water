#pragma once

struct TextureDescriptor {
  std::string uniformName{};
  GLuint unit = 0;
  GLenum target         = GL_TEXTURE_2D;
  GLenum internalFormat = GL_RGB8;          // Color format in the OpenGL
  GLenum format         = GL_RGB;           // Color format of the loaded image
  GLenum type           = GL_UNSIGNED_BYTE; // Color bytes format of the loaded image
  GLenum minFilter      = GL_LINEAR_MIPMAP_LINEAR;
  GLenum magFilter      = GL_LINEAR_MIPMAP_LINEAR;
  GLenum wrapS          = GL_REPEAT;
  GLenum wrapT          = GL_REPEAT;
  GLenum wrapR          = GL_REPEAT;
  bool genMipMap        = true;
};

