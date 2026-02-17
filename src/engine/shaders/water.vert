#version 460 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTex;

out vec4 v_worldPos;

uniform mat4 u_camPV;
uniform mat4 u_model;
uniform vec3 u_camPos;
uniform float u_time;
uniform float u_wavelength;
uniform float u_speed;
uniform float u_amplitude;
uniform float u_persistence;
uniform float u_lacunarity;
uniform float u_speedMul;
uniform float u_dragMul;
uniform int u_count;

float getWave(vec2 coord) {
  float twoOverWavelength = 2.f / u_wavelength;
  float totalWave = 0.f;
  float totalWeight = 0.f;

  float alpha = u_amplitude;
  float omega = twoOverWavelength;           // Frequency
  float phase = u_speed * twoOverWavelength; // Move
  float iter = 0.f;
  float weight = 1.f;

  for (int i = 0; i < u_count; i++) {
    vec2 dir = vec2(sin(iter), cos(iter));
    float angle = dot(dir, coord) * omega + u_time * phase;
    float sharpWave = exp(sin(angle) - 1.f);
    float wave = alpha * sharpWave * weight;

    totalWave += wave;
    totalWeight += weight;

    float derivativeFactor = wave * omega * cos(angle);
    vec2 slope = dir * derivativeFactor * weight * u_dragMul;

    alpha *= u_persistence;
    omega *= u_lacunarity;
    phase *= u_speedMul;
    weight = mix(weight, 0.f, 0.2f);

    coord += slope;
    iter += 1232.399963f;
  }

  return totalWave / totalWeight;
}

void main() {
  v_worldPos = u_model * vec4(inPos, 1.f);
  v_worldPos.xz += u_camPos.xz;
  v_worldPos.y += getWave(v_worldPos.xz);

  gl_Position = u_camPV * v_worldPos;
}

