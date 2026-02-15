#pragma once

#include "Shader.hpp"
struct Fog {
  vec3 color;
  float density;

  void setUniforms(Shader& shader) {
    shader.setUniform3f("u_fogColor", color);
    shader.setUniform1f("u_fogDensity", density);
  }
};

