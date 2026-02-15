#version 460

in vec2 texCoord;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_sceneColorTex;
layout(binding = 1) uniform sampler2D u_sceneDepthTex;

uniform vec3 u_fogColor;
uniform float u_camNear;
uniform float u_camFar;
uniform float u_fogDensity;

float linearizeDepth(float depth) {
  float z = depth * 2.f - 1.f;
  return (2.f * u_camNear * u_camFar) / (u_camFar + u_camNear - z * (u_camFar - u_camNear));
}

void main() {
  vec3 col = texture(u_sceneColorTex, texCoord).rgb;
  // float depthNonLinear = texture(u_sceneDepthTex, texCoord).r;
  // float depth = linearizeDepth(depthNonLinear) / u_camFar;

  // float fogFactor = 1.f - exp(-depth * u_fogDensity);
  // fogFactor = clamp(fogFactor, 0.f, 1.f);

  // col = mix(col, u_fogColor, fogFactor);

  FragColor = vec4(col, 1.f);
}

