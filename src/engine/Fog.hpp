#pragma once

#include "Shader.hpp"

struct Fog {
  vec3 color;
  float density;
  float start;
  float heightFalloff;

  void setUniforms(Shader& shader) {
    shader.setUniform3f("u_fogColor", color);
    shader.setUniform1f("u_fogDensity", density);
    shader.setUniform1f("u_fogStart", start);
    shader.setUniform1f("u_fogHeightFalloff", heightFalloff);
  }
};

