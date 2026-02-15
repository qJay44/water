#version 460 core
#define MAX_DIRS 32

out vec4 FragColor;

in vec4 worldPos;
in vec3 preWaveWorldPos;

uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform vec3 u_camPos;
uniform float u_time;
uniform float u_wavelength;
uniform float u_speed;
uniform float u_amplitude;
uniform float u_persistence;
uniform float u_lacunarity;
uniform float u_speedMul;
uniform float a;
uniform float b;
uniform int u_count; // NOTE: Up to MAX_DIRS

uniform vec2 u_dirs[MAX_DIRS]; // D[i]

vec3 lightDir = normalize(u_lightPos - worldPos.xyz);
vec3 viewDir = normalize(u_camPos - worldPos.xyz);

vec3 directionalLight(vec3 normal) {
  float diffuse = max(dot(normal, lightDir), 0.f);
  float ambient = 0.1f;

  return u_lightColor * (diffuse + ambient);
}

vec3 specularLight(vec3 normal) {
  vec3 halfwayDir = normalize(lightDir + viewDir);

  float specular = pow(max(dot(normal, halfwayDir), 0.f), 32);

  return u_lightColor * specular;
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
  vec3 col = vec3(0.07f, 0.13f, 0.17f);
  vec3 waveData = waveSurfaceExp(preWaveWorldPos.xz);
  vec3 normal = normalize(vec3(-waveData.y, 1.f, -waveData.z));

  col *= directionalLight(normal);
  col += specularLight(normal);

  FragColor = vec4(col, 1.f);
}

