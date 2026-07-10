#pragma once

#include "../engine/Shader.hpp"
#include "../engine/texture/Texture2D.hpp"
#include "global.hpp"
#include "nlohmann/json.hpp"
#include "general.hpp"

namespace water {

// Sum of sines approximation (modified)
struct SOSA {
  float worldSize = 1000.f;
  float wavelength = 1.f;
  float amplitude = 1.f;
  float speed = 1.f;
  float persistence = 0.5f; // Amplitude multipplier (should go lower with each wave)
  float lacunarity = 2.f;   // Frequency multipplier (should go higher with each wave)
  float speedMul = 1.3f;    // Speed multipplier (slightly faster each (smaller) wave)
  float dragMul = 0.38f;    // How much waves pull on the water
  int waves = 1;

  Shader shader = Shader("water/sosa.comp");
  Texture2D texNormheight = Texture2D(ivec2{texResolution}, {.internalFormat = GL_RGBA32F, .format = GL_RGBA});

  void update() {
    shader.use();
    shader.setUniform1f("u_worldSize", worldSize);
    shader.setUniform1f("u_wavelength", wavelength);
    shader.setUniform1f("u_speed", speed);
    shader.setUniform1f("u_amplitude", amplitude);
    shader.setUniform1f("u_persistence", persistence);
    shader.setUniform1f("u_lacunarity", lacunarity);
    shader.setUniform1f("u_speedMul", speedMul);
    shader.setUniform1f("u_dragMul", dragMul);
    shader.setUniform1f("u_time", global::time);
    shader.setUniform1i("u_count", waves);
    glBindImageTexture(0, texNormheight.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(numWorkGroups, numWorkGroups, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SOSA,
  wavelength,
  amplitude,
  speed,
  persistence,
  lacunarity,
  speedMul,
  dragMul,
  waves,
  worldSize
);

} // namespace water

