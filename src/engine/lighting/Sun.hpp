#pragma once

#include "../Shader.hpp"
#include "../Camera.hpp"
#include "../mesh/Mesh.hpp"

struct Sun {
  float focus;
  float intensity;
  vec3 dir; // Looking towards [dir]
  vec3 color{1.f};

  vec3 skyHorizonColor = vec3(1.f);
  vec3 skyZenithColor{0.289f, 0.565f, 1.f};
  vec3 groundColor = vec3(0.637f);

  void setUniforms(Shader& shader) {
    shader.setUniform1f("u_sunFocus", focus);
    shader.setUniform1f("u_sunIntensity", intensity);
    shader.setUniform3f("u_sunDir", dir);
    shader.setUniform3f("u_sunColor", color);
    shader.setUniform3f("u_skyHorizonColor", skyHorizonColor);
    shader.setUniform3f("u_skyZenithColor", skyZenithColor);
    shader.setUniform3f("u_groundColor", groundColor);
  }

  void draw(const Camera* cam, Shader& shader) {
    setUniforms(shader);
    shader.setUniformMatrix4f("u_camInv", cam->getProjViewInv());

    Mesh::screenDraw(cam, shader);
  }
};

