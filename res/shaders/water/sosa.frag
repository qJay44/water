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

layout(binding = 0) uniform sampler2D u_texNormheight;
layout(binding = 1) uniform samplerCube u_texSkybox;

vec3 getReflection(vec3 reflDir) {
  float sun = pow(max(0, dot(reflDir, u_lightDir)), u_sunFocus) * u_sunIntensity;
  vec3 skybox = texture(u_texSkybox, reflDir).rgb;

  return skybox + sun;
}

void main() {
  // 1. Directions
  vec3 normal = normalize(texture(u_texNormheight, v_uv).rgb);
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
  vec3 reflCol = getReflection(reflDir);

  // 5. COMBINE using Fresnel
  float fresnel = pow(1.f - vDotN, 4.f);
  vec3 finalCol = mix(diffuseCol + scatterCol, reflCol, fresnel);

  // 6. Specular (The Sun Glint)
  float specAmount = pow(max(dot(normal, halfwayDir), 0.f), 128.f);
  vec3 specularCol = u_lightColor * specAmount * u_sunIntensity;

  // Add the sun on top (Additive)
  finalCol += specularCol;

  FragColor = vec4(finalCol, 1.f);
}

