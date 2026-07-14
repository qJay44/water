#pragma once

#include "../engine/Shader.hpp"
#include "../engine/texture/Texture2D.hpp"
#include "glm/exponential.hpp"
#include "glm/trigonometric.hpp"
#include <utility>

namespace water {

struct FFT {
  float worldSize = 1024.f;

  // gridPoints*gridPoints -> M x N
  // NOTE: Keep this in power of 2
  int gridPoints = 1024;

  GLuint numWorkGroups = gridPoints / 16;
  float seed1 = 13.37f;
  float seed2 = 42.f;

  float amplitude = 5e-4f; // A
  float windSpeed = 30.f;  // V
  float g = 9.81f;
  float windAngle = glm::radians(45.f);
  float waterDepth = 1.f; // D

  float choppinessControl = 1.f;

  Shader shaderNoise{"water/fft/noise.comp"};
  Shader shaderWaveHieghtField{"water/fft/heightfield.comp"};
  Shader shaderWaveHieghtFieldTimeEvolution{"water/fft/heightfield_time.comp"};
  Shader shaderIFFT_Horizontal{"water/fft/ifft_horizontal.comp"};
  Shader shaderIFFT_Vertical{"water/fft/ifft_vertical.comp"};
  Shader shaderDisplacement{"water/fft/displacement.comp"};

  Texture2D texNoise;
  Texture2D texWaveHeightField;
  Texture2D texWaveHeightFieldTimeEvolution;
  Texture2D texWaveChoppyX;
  Texture2D texWaveChoppyZ;
  Texture2D texWaveHeightFieldTimeEvolution_IFFT;
  Texture2D texWaveChoppyX_IFFT;
  Texture2D texWaveChoppyZ_IFFT;
  Texture2D texPing;
  Texture2D texPong;
  Texture2D texDisplacement;

  FFT() {
    rebuild();
  }

  // Call on parameters change
  void generateNoise() {
    shaderNoise.use();
    shaderNoise.setUniform1f("u_seed1", seed1);
    shaderNoise.setUniform1f("u_seed2", seed2);
    glBindImageTexture(0, texNoise.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    glDispatchCompute(numWorkGroups, numWorkGroups, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }

  // Call on parameters change
  void generateWaveHeightField() {
    vec2 windDir{cos(windAngle), sin(windAngle)}; // w (unit vector)
    float L = (windSpeed * windSpeed) / g;

    texNoise.bind(0);

    shaderWaveHieghtField.use();
    shaderWaveHieghtField.setUniform1f("u_worldSize", worldSize);
    shaderWaveHieghtField.setUniform1f("u_amplitude", amplitude);
    shaderWaveHieghtField.setUniform1f("u_L", L);
    shaderWaveHieghtField.setUniform1f("u_g", g);
    shaderWaveHieghtField.setUniform1f("u_waterDepth", waterDepth);
    shaderWaveHieghtField.setUniform2f("u_windDir", windDir);
    glBindImageTexture(0, texNoise.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(1, texWaveHeightField.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(numWorkGroups, numWorkGroups, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }

  // Call every frame
  void generateWaveHeightFieldTimeEvolution() {
    shaderWaveHieghtFieldTimeEvolution.use();
    shaderWaveHieghtFieldTimeEvolution.setUniform1f("u_worldSize", worldSize);
    shaderWaveHieghtFieldTimeEvolution.setUniform1f("u_time", global::time);
    glBindImageTexture(0, texWaveHeightField.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, texWaveHeightFieldTimeEvolution.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    glBindImageTexture(2, texWaveChoppyX.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    glBindImageTexture(3, texWaveChoppyZ.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    glDispatchCompute(numWorkGroups, numWorkGroups, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }

  // Call every frame
  void generateIFFT(const Texture2D& base, Texture2D& result) {
    glCopyImageSubData(
      base.getId(), GL_TEXTURE_2D, 0, 0, 0, 0,
      texPing.getId(), GL_TEXTURE_2D, 0, 0, 0, 0,
      gridPoints, gridPoints, 1
    );

    GLuint ping = texPing.getId();
    GLuint pong = texPong.getId();

    shaderIFFT_Horizontal.use();
    for (int stage = 1; stage < gridPoints; stage <<= 1) {
      shaderIFFT_Horizontal.setUniform1i("u_stage", stage);
      glBindImageTexture(0, ping, 0, GL_FALSE, 0, GL_READ_ONLY , GL_RG32F);
      glBindImageTexture(1, pong, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
      glDispatchCompute(numWorkGroups, numWorkGroups, 1);
      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

      std::swap(ping, pong);
    }

    shaderIFFT_Vertical.use();
    for (int stage = 1; stage < gridPoints - 1; stage <<= 1) {
      shaderIFFT_Vertical.setUniform1i("u_stage", stage);
      glBindImageTexture(0, pong, 0, GL_FALSE, 0, GL_READ_ONLY , GL_RG32F);
      glBindImageTexture(1, ping, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
      glDispatchCompute(numWorkGroups, numWorkGroups, 1);
      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

      std::swap(ping, pong);
    }

    glCopyImageSubData(
      texPing.getId(), GL_TEXTURE_2D, 0, 0, 0, 0,
      result.getId(), GL_TEXTURE_2D, 0, 0, 0, 0,
      gridPoints, gridPoints, 1
    );
  }

  // Call every frame
  void generateDisplacement() {
    shaderDisplacement.use();
    shaderDisplacement.setUniform1f("u_choppinessControl", choppinessControl);
    glBindImageTexture(0, texWaveHeightFieldTimeEvolution_IFFT.getId(), 0, GL_FALSE, 0, GL_READ_ONLY , GL_RG32F);
    glBindImageTexture(1, texWaveChoppyX_IFFT.getId(), 0, GL_FALSE, 0, GL_READ_ONLY , GL_RG32F);
    glBindImageTexture(2, texWaveChoppyZ_IFFT.getId(), 0, GL_FALSE, 0, GL_READ_ONLY , GL_RG32F);
    glBindImageTexture(3, texDisplacement.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(numWorkGroups, numWorkGroups, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }

  void update() {
    generateWaveHeightFieldTimeEvolution();
    generateIFFT(texWaveHeightFieldTimeEvolution, texWaveHeightFieldTimeEvolution_IFFT);
    generateIFFT(texWaveChoppyX, texWaveChoppyX_IFFT);
    generateIFFT(texWaveChoppyZ, texWaveChoppyZ_IFFT);
    generateDisplacement();
  }

  void rebuild() {
    ivec2 size{gridPoints};
    TextureDescriptor descRG{.internalFormat = GL_RG32F, .format = GL_RG, .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
    TextureDescriptor descRGBA{.internalFormat = GL_RGBA32F, .format = GL_RGBA, .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};

    numWorkGroups = gridPoints / 16;
    texNoise                             = Texture2D{size, descRG};
    texWaveHeightField                   = Texture2D{size, descRGBA};
    texWaveHeightFieldTimeEvolution      = Texture2D{size, descRG};
    texWaveChoppyX                       = Texture2D{size, descRG};
    texWaveChoppyZ                       = Texture2D{size, descRG};
    texWaveHeightFieldTimeEvolution_IFFT = Texture2D{size, descRG};
    texWaveChoppyX_IFFT                  = Texture2D{size, descRG};
    texWaveChoppyZ_IFFT                  = Texture2D{size, descRG};
    texPing                              = Texture2D{size, descRG};
    texPong                              = Texture2D{size, descRG};
    texDisplacement                      = Texture2D{size, descRGBA};

    generateNoise();
    generateWaveHeightField();
  }
};

} // namespace water

