#pragma once

#include "mesh/Mesh.hpp"

class Light : public Mesh {
public:
  Light(vec3 position, float radius = 30.f, vec3 color = {1.f, 1.f, 1.f});

  const vec3& getPosition() const;
  const vec3& getColor() const;

  void update();
  void setUniforms(Shader& shader) const;

  void draw(const Camera* camera, Shader& shader, bool forceNoWireframe = false) const;

private:
  friend struct gui;

  vec3 position;
  float radius;
  vec3 color;
};

