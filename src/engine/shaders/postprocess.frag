#version 460

in vec2 v_uv;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_sceneColorTex;
layout(binding = 1) uniform sampler2D u_sceneDepthTex;

uniform mat4 u_camInv;
uniform vec3 u_camPos;
uniform vec3 u_fogColor;
uniform float u_camNear;
uniform float u_camFar;
uniform float u_fogThinness;
uniform float u_fogStart;

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
  vec3 col = texture(u_sceneColorTex, v_uv).rgb;
  float depthRaw = texture(u_sceneDepthTex, v_uv).r;

  vec3 worldPos = getWorldPosFromDepth(depthRaw, v_uv);
  vec3 viewDir = normalize(worldPos - u_camPos);
  float horizonMask = 1.f - abs(viewDir.y);
  horizonMask = pow(horizonMask, u_fogThinness);

  float dist = distance(u_camPos, worldPos);
  float distFactor = clamp(dist / u_fogStart, 0.f, 1.f);

  float fogT = horizonMask * distFactor;
  vec3 finalColor = mix(col, u_fogColor, fogT);

  FragColor = vec4(finalColor, 1.f);
}

