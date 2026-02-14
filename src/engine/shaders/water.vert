#version 460 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTex;

out vec4 worldPos;
out vec3 normal;

uniform mat4 u_camPV;
uniform mat4 u_model;
uniform float u_time;
uniform float u_wavelength;
uniform float u_speed;
uniform float u_amplitude;
uniform float u_persistence;
uniform float u_lacunarity;
uniform float u_speedMul;
uniform int u_count; // NOTE: Up to D length

const float epsilon = 0.01f;

const vec2 D[5] = vec2[5](
    vec2(1.0, 0.0),    // Wave 0: Primary direction (Directly X)
    vec2(0.8, 0.6),    // Wave 1: Large secondary angle (~37°)
    vec2(0.9, -0.4),   // Wave 2: Subtle counter-angle (~ -24°)
    vec2(0.5, 0.8),    // Wave 3: Steep cross-current detail (~58°)
    vec2(0.7, -0.7)    // Wave 4: Diagonal choppy peaks (~ -45°)
);

float wave(vec2 coord) {
  float twoOverWavelength = 2.f / u_wavelength;
  float res = 0.f;

  float alpha = u_amplitude;
  float omega = twoOverWavelength;           // Frequency
  float phase = u_speed * twoOverWavelength; // Move

  for (int i = 0; i < u_count; i++) {
    res += alpha * sin(dot(D[i], coord) * omega + u_time * phase);

    alpha *= u_persistence;
    omega *= u_lacunarity;
    phase *= u_speedMul;
  }

  return res;
}

void main() {
  worldPos = u_model * vec4(inPos, 1.f);

  // Probably will be wrong if plane is rotated
  float heightCenter = wave(worldPos.xz);
  float heightRight = wave(worldPos.xz + vec2(epsilon, 0.f));
  float heightUp = wave(worldPos.xz + vec2(0.f, epsilon));

  float dydx = (heightRight - heightCenter) / epsilon;
  float dydz = (heightUp - heightCenter) / epsilon;
  normal = normalize(vec3(-dydx, 1.f, -dydz));

  worldPos.y += heightCenter;

  gl_Position = u_camPV * worldPos;
}

