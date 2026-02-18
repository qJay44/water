#version 460 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTex;

out vec4 v_worldPos;
out vec3 v_viewVec;

uniform mat4 u_camPV;
uniform mat4 u_camInv;
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

// Returns vec3(wave, slopeX, slopeZ)
vec3 getWaveSurface(vec2 coord) {
  float twoOverWavelength = 2.f / u_wavelength;
  float totalWave = 0.f;
  vec2 totalSlope = vec2(0.f);

  float alpha = u_amplitude;
  float omega = twoOverWavelength;           // Frequency
  float phase = u_speed * twoOverWavelength; // Move

  for (int i = 0; i < u_count; i++) {
    float angleStep = float(i) * 2.39996f;
    vec2 dir = vec2(sin(angleStep), cos(angleStep));

    float angle = dot(dir, coord) * omega + u_time * phase;
    float sharpWave = exp(sin(angle) - 1.f);
    float wave = alpha * sharpWave;

    totalWave += wave;

    float derivativeFactor = wave * omega * cos(angle);
    vec2 slope = dir * derivativeFactor;

    alpha *= u_persistence;
    omega *= u_lacunarity;
    phase *= u_speedMul;

    coord += slope * u_dragMul;
    totalSlope += slope;
  }

  return vec3(totalWave, totalSlope);
}

void main() {
  v_worldPos = u_model * vec4(inPos, 1.f);
  // v_worldPos.xz += u_camPos.xz;
  v_worldPos.y += getWaveSurface(v_worldPos.xz).x;
  v_viewVec = u_camPos - v_worldPos.xyz;

  gl_Position = u_camPV * v_worldPos;
}

