#pragma once

#include "Shader.hpp"

struct Fog {
  vec3 color;
  float thinness;
  float start;

  void setUniforms(Shader& shader) {
    shader.setUniform3f("u_fogColor", color);
    shader.setUniform1f("u_fogThinness", thinness);
    shader.setUniform1f("u_fogStart", start);
  }
};

