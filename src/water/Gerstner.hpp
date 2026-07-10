#pragma once

#include "../engine/Shader.hpp"
#include "../engine/texture/Texture2D.hpp"
#include "global.hpp"
#include "nlohmann/json.hpp"
#include "general.hpp"

namespace water {

struct Gerstner {
  float worldSize = 1000.f;
  float wavelength = 1.f; // lambda to k_magnitude = 2pi/lambda
  float amplitude = 1.f;  // A
  float frequency = 1.f;  // omega
  float phase = 1.f;      // phi
  int N = 1;

  float wavelengthStep = 1.f;
  float amplitudeStep = 1.f;
  float frequencyStep = 1.f;
  float phaseStep = 1.f;
  float angleStep = 1.f;  // rotate k_vector by angleStep

  Shader shader = Shader("water/gerstner.comp");
  Texture2D texDisplacement = Texture2D(ivec2{texResolution}, {.internalFormat = GL_RGBA32F, .format = GL_RGBA});
  Texture2D texNormal = Texture2D(ivec2{texResolution}, {.internalFormat = GL_RGBA32F, .format = GL_RGBA});

  void update() {
    shader.use();

    shader.setUniform1f("u_worldSize", worldSize);
    shader.setUniform1f("u_wavelength", wavelength);
    shader.setUniform1f("u_amplitude", amplitude);
    shader.setUniform1f("u_frequency", frequency);
    shader.setUniform1f("u_phase", phase);
    shader.setUniform1i("u_N", N);

    shader.setUniform1f("u_wavelengthStep", wavelengthStep);
    shader.setUniform1f("u_amplitudeStep", amplitudeStep);
    shader.setUniform1f("u_frequencyStep", frequencyStep);
    shader.setUniform1f("u_phaseStep", phaseStep);
    shader.setUniform1f("u_angleStep", angleStep);
    shader.setUniform1f("u_time", global::time);

    glBindImageTexture(0, texDisplacement.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, texNormal.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(numWorkGroups, numWorkGroups, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }

  void rebuild() {
    texDisplacement = Texture2D(ivec2{texResolution}, {.internalFormat = GL_RGBA32F, .format = GL_RGBA});
    texNormal = Texture2D(ivec2{texResolution}, {.internalFormat = GL_RGBA32F, .format = GL_RGBA});
  }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Gerstner,
  worldSize,
  wavelength,
  amplitude,
  frequency,
  phase,
  N,
  wavelengthStep,
  amplitudeStep,
  frequencyStep,
  phaseStep,
  angleStep
);

} // namespace water

