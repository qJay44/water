#version 460 core

#define MAX_DIRS 32

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTex;

out vec4 worldPos;
out vec3 preWaveWorldPos;
out vec3 normal2;

uniform mat4 u_camPV;
uniform mat4 u_model;
uniform float u_time;
uniform float u_wavelength;
uniform float u_speed;
uniform float u_amplitude;
uniform float u_persistence;
uniform float u_lacunarity;
uniform float u_speedMul;
uniform int u_count; // NOTE: Up to MAX_DIRS

uniform vec2 u_dirs[MAX_DIRS]; // D[i]

float wave(vec2 coord) {
  float twoOverWavelength = 2.f / u_wavelength;
  float res = 0.f;

  float alpha = u_amplitude;
  float omega = twoOverWavelength;           // Frequency
  float phase = u_speed * twoOverWavelength; // Move

  for (int i = 0; i < u_count; i++) {
    res += alpha * sin(dot(u_dirs[i], coord) * omega + u_time * phase);

    alpha *= u_persistence;
    omega *= u_lacunarity;
    phase *= u_speedMul;
  }

  return res;
}

float waveExp(vec2 coord) {
  float twoOverWavelength = 2.f / u_wavelength;
  float res = 0.f;

  float alpha = u_amplitude;
  float omega = twoOverWavelength;           // Frequency
  float phase = u_speed * twoOverWavelength; // Move

  for (int i = 0; i < u_count; i++) {
    res += alpha * exp(sin(dot(u_dirs[i], coord) * omega + u_time * phase) - 1.f);

    alpha *= u_persistence;
    omega *= u_lacunarity;
    phase *= u_speedMul;
  }

  return res;
}

// Returns vec3(wave, slopeX, slopeZ)
vec3 waveSurface(vec2 coord) {
  float twoOverWavelength = 2.f / u_wavelength;
  float wave = 0.f;
  vec2 dxdz = vec2(0.f);

  float alpha = u_amplitude;
  float omega = twoOverWavelength;           // Frequency
  float phase = u_speed * twoOverWavelength; // Move

  for (int i = 0; i < u_count; i++) {
    float angle = dot(u_dirs[i], coord) * omega + u_time * phase;
    float sinAngle = sin(angle);
    float cosAngle = cos(angle);

    wave += alpha * sinAngle;

    float derivativeFactor = alpha * omega * cosAngle;
    dxdz += u_dirs[i] * derivativeFactor;

    alpha *= u_persistence;
    omega *= u_lacunarity;
    phase *= u_speedMul;
  }

  return vec3(wave, dxdz);
}

// Returns vec3(wave, slopeX, slopeZ)
vec3 waveSurfaceExp(vec2 coord) {
  float twoOverWavelength = 2.f / u_wavelength;
  float wave = 0.f;
  vec2 dxdz = vec2(0.f);

  float alpha = u_amplitude;
  float omega = twoOverWavelength;           // Frequency
  float phase = u_speed * twoOverWavelength; // Move

  for (int i = 0; i < u_count; i++) {
    vec2 dir = u_dirs[i];
    float angle = dot(dir, coord) * omega + u_time * phase;
    float sinAngle = sin(angle);
    float cosAngle = cos(angle);
    float sharpWave = exp(sinAngle - 1.f);

    wave += alpha * sharpWave;

    float derivativeFactor = alpha * omega * cosAngle * sharpWave;
    dxdz += dir * derivativeFactor;

    alpha *= u_persistence;
    omega *= u_lacunarity;
    phase *= u_speedMul;

    coord += dxdz;
  }

  return vec3(wave, dxdz);
}

void main() {
  worldPos = u_model * vec4(inPos, 1.f);
  preWaveWorldPos = worldPos.xyz;

  vec3 waveData = waveSurfaceExp(worldPos.xz);

  worldPos.y += waveData.x;

  gl_Position = u_camPV * worldPos;
}

