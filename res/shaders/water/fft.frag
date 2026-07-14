#version 460 core

out vec4 FragColor;

in vec3 v_worldPos;
in vec3 v_viewVec;
in vec2 v_uv;

uniform vec3 u_camPos;
uniform vec3 u_lightColor;
uniform vec3 u_lightDir;
uniform float u_sunFocus;
uniform float u_sunIntensity;

layout(binding = 0) uniform sampler2D u_texDisplacement;
layout(binding = 1) uniform sampler2D u_texNormal;
layout(binding = 2) uniform samplerCube u_texSkybox;

void main() {
  // 1. Directions
  // vec3 normal = normalize(texture(u_texNormal, v_uv).rgb);
  vec3 normal = normalize(texture(u_texDisplacement, v_uv).rgb);
  vec3 viewDir = normalize(v_viewVec);
  vec3 reflDir = reflect(-viewDir, normal);
  vec3 halfwayDir = normalize(u_lightDir + viewDir);
  float vDotN = max(dot(viewDir, normal), 0.f);
  float lDotN = max(dot(u_lightDir, normal), 0.f);

  // 2. Scatter light
  float scatter = pow(max(dot(viewDir, -u_lightDir), 0.f), 2.f) * (1.f - vDotN);
  vec3 scatterCol = vec3(0.f, 0.4f, 0.4f) * scatter * u_sunIntensity;

  // 3. Diffuse base color
  vec3 waterBase = vec3(0.02f, 0.1f, 0.2f);
  vec3 diffuseCol = waterBase * (lDotN + 0.2f);

  // 4. Reflection
  vec3 reflCol = texture(u_texSkybox, reflDir).rgb;

  // 5. COMBINE using Fresnel
  float f0 = 0.02f;
  float fresnel = f0 + (1.f - f0) * pow(1.f - vDotN, 5.f);
  vec3 finalCol = mix(diffuseCol + scatterCol, reflCol, fresnel);

  // 6. Specular (The Sun Glint)
  float specAmount = pow(max(dot(normal, halfwayDir), 0.f), u_sunFocus);
  vec3 specularCol = u_lightColor * specAmount * u_sunIntensity;

  // Add the sun on top (Additive)
  finalCol += specularCol;
  // finalCol = pow(finalCol, vec3(1.f / 2.2f));

  FragColor = vec4(finalCol, 1.f);
}

