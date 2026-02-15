#pragma once

#include <fstream>
#include <string_view>

#include "Shader.hpp"
#include "mesh/meshes.hpp"
#include "nlohmann/json.hpp"

#define WATER_MAX_DIRS 32

struct Water : public Mesh {
  int resolution;
  float scale;

  float wavelength = 1.f;
  float amplitude = 1.f;
  float speed = 1.f;
  float persistence = 0.5f; // Amplitude multipplier (should go lower with each wave)
  float lacunarity = 2.f;   // Frequency multipplier (should go higher with each wave)
  float speedMul = 1.3f;    // Speed multipplier (slightly faster each (smaller) wave)
  int waves = 1;

  vec2 dirs[WATER_MAX_DIRS];
  float timeOffsets[WATER_MAX_DIRS];

  Water(int resolution, float scale)
    : Mesh(meshes::plane(resolution)), resolution(resolution), scale(scale)
  {
    setScale({scale * 0.5f, 0.f, scale * 0.5f});
    randomizeDirs();
  }

  void loadPreset(std::string_view name) {
    using json = nlohmann::json;

    fspath path = fspath("res/data/preset") / name;

    std::ifstream f(path);

    if (f.is_open()) {
      json data = json::parse(f);

      resolution  = data["resolution"];
      scale       = data["scale"];

      wavelength  = data["wavelength"];
      amplitude   = data["amplitude"];
      speed       = data["speed"];
      persistence = data["persistence"];
      lacunarity  = data["lacunarity"];
      speedMul    = data["speedMul"];
      waves       = data["waves"];

      rebuild();

      f.close();
    } else {
      warning("[Water::loadPreset] Could not open the file [{}]", path.string());
    }
  }

  void savePreset(std::string_view name) const {
    using json = nlohmann::json;

    fspath path = fspath("res/data/preset") / name;

    std::ofstream f(path);

    if (f.is_open()) {
      json data;

      data["resolution"]  = resolution;
      data["scale"]       = scale;

      data["wavelength"]  = wavelength;
      data["amplitude"]   = amplitude;
      data["speed"]       = speed;
      data["persistence"] = persistence;
      data["lacunarity"]  = lacunarity;
      data["speedMul"]    = speedMul;
      data["waves"]       = waves;

      f << std::setw(2) << data << std::endl;
      f.close();
    } else {
      error("[Water::savePreset] Could not open the file [{}]", path.string());
    }
  }

  void setUniforms(Shader& shader) {
    shader.setUniform1f("u_wavelength", wavelength);
    shader.setUniform1f("u_speed", speed);
    shader.setUniform1f("u_amplitude", amplitude);
    shader.setUniform1f("u_persistence", persistence);
    shader.setUniform1f("u_lacunarity", lacunarity);
    shader.setUniform1f("u_speedMul", speedMul);
    shader.setUniform1i("u_count", waves);
    shader.setUniform2fv("u_dirs", WATER_MAX_DIRS, (float*)(dirs));
  }

  void randomizeDirs() {
    for (int i = 0; i < WATER_MAX_DIRS; i++) {
      float angle = float(i);
      dirs[i] = {cos(angle), sin(angle)};
    }
  }

  void rebuild() {
    static_cast<Mesh&>(*this) = Mesh(meshes::plane(resolution));
    setScale({scale * 0.5f, 0.f, scale * 0.5f});
  }
};

