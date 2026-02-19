#version 460 core

out vec4 FragColor;

in vec4 v_worldPos;
in vec3 v_viewVec;

layout (binding = 2) uniform samplerCube u_skyboxTex;

uniform vec3 u_camPos;
uniform vec3 u_sunColor;
uniform vec3 u_sunDir;
uniform float u_camFar;
uniform float u_sunFocus;
uniform float u_sunIntensity;
uniform float u_time;
uniform float u_wavelength;
uniform float u_speed;
uniform float u_amplitude;
uniform float u_persistence;
uniform float u_lacunarity;
uniform float u_speedMul;
uniform float u_dragMul;
uniform int u_count;

vec3 getReflection(vec3 reflDir) {
  float sun = pow(max(0, dot(reflDir, -u_sunDir)), u_sunFocus) * u_sunIntensity;
  vec3 skybox = texture(u_skyboxTex, reflDir).rgb;

  return skybox + sun;
}

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

    coord += slope * u_dragMul;
    totalSlope += slope;

    alpha *= u_persistence;
    omega *= u_lacunarity;
    phase *= u_speedMul;
  }

  return vec3(totalWave, totalSlope);
}

void main() {
  // 1. Directions
  vec3 lightDir = -u_sunDir;
  vec3 waveData = getWaveSurface(v_worldPos.xz);
  vec3 normal = normalize(vec3(-waveData.y, 1.f, -waveData.z));
  vec3 viewDir = normalize(v_viewVec);
  vec3 reflDir = reflect(-viewDir, normal);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float vDotN = max(dot(viewDir, normal), 0.f);
  float lDotN = max(dot(lightDir, normal), 0.f);

  // 2. Scatter light
  float scatter = pow(max(dot(viewDir, u_sunDir), 0.f), 2.f) * (1.f - vDotN);
  vec3 scatterCol = vec3(0.f, 0.4f, 0.4f) * scatter * u_sunIntensity;

  // 3. Diffuse base color
  vec3 waterBase = vec3(0.02f, 0.1f, 0.2f);
  vec3 diffuseCol = waterBase * (lDotN + 0.2f);

  // 4. Reflection
  vec3 reflCol = getReflection(reflDir);

  // 5. COMBINE using Fresnel
  float fresnel = pow(1.f - vDotN, 4.f);
  vec3 finalCol = mix(diffuseCol + scatterCol, reflCol, fresnel);

  // 6. Specular (The Sun Glint)
  float specAmount = pow(max(dot(normal, halfwayDir), 0.f), 128.f);
  vec3 specularCol = u_sunColor * specAmount * u_sunIntensity;

  // Add the sun on top (Additive)
  finalCol += specularCol;

  FragColor = vec4(finalCol, 1.f);
}

