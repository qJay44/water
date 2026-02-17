#version 460 core

out vec4 FragColor;

in vec4 v_worldPos;

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
uniform float u_dragMul;
uniform int u_count;

vec3 lightDir = normalize(u_lightPos - v_worldPos.xyz);
vec3 viewDir = normalize(u_camPos - v_worldPos.xyz);

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
vec3 getWaveSurface(vec2 coord) {
  float twoOverWavelength = 2.f / u_wavelength;
  float totalWave = 0.f;
  float totalWeight = 0.f;
  vec2 totalSlope = vec2(0.f);

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
    totalSlope += slope;
    iter += 1232.399963f;
  }

  totalWave /= totalWeight;

  return vec3(totalWave, totalSlope);
}

void main() {
  vec3 col = vec3(0.07f, 0.13f, 0.17f);
  vec3 waveData = getWaveSurface(v_worldPos.xz);
  vec3 normal = normalize(vec3(-waveData.y, 1.f, -waveData.z));

  col += specularLight(normal);
  col *= directionalLight(normal);

  FragColor = vec4(col, 1.f);
}

