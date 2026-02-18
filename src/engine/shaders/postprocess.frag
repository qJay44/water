#version 460

in vec2 v_tex;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_sceneColorTex;
layout(binding = 1) uniform sampler2D u_sceneDepthTex;

uniform mat4 u_camInv;
uniform vec3 u_camPos;
uniform vec3 u_fogColor;
uniform float u_camNear;
uniform float u_camFar;
uniform float u_fogDensity;
uniform float u_fogStart;
uniform float u_fogHeightFalloff;

float linearizeDepth(float depth) {
  float z = depth * 2.f - 1.f;
  return (2.f * u_camNear * u_camFar) / (u_camFar + u_camNear - z * (u_camFar - u_camNear));
}

vec3 getWorldPosFromDepth(float depth, vec2 uv) {
  float z = depth * 2.f - 1.f;
  vec4 clipPos = vec4(uv * 2.f - 1.f, z, 1.f);
  vec4 worldPos = u_camInv * clipPos;
  worldPos /= worldPos.w;

  return worldPos.xyz;
}

void main() {
  vec3 col = texture(u_sceneColorTex, v_tex).rgb;

  FragColor = vec4(col, 1.f);
}

