#version 460

in vec3 v_viewVec;

out vec4 FragColor;

uniform vec3 u_camPos;
uniform vec3 u_sunColor;
uniform vec3 u_sunDir;
uniform vec3 u_groundColor;
uniform vec3 u_skyHorizonColor;
uniform vec3 u_skyZenithColor;
uniform float u_sunFocus;
uniform float u_sunIntensity;

vec3 getEnvironmentLight(vec3 viewDir) {
  float skyGradientT = pow(smoothstep(0.f, 0.4f, viewDir.y), 0.35f);
  vec3 skyGradient = mix(u_skyHorizonColor, u_skyZenithColor, skyGradientT);
  float sun = pow(max(0, dot(viewDir, -u_sunDir)), u_sunFocus) * u_sunIntensity;

  float groundToSkyT = smoothstep(-0.01f, 0.f, viewDir.y);
  float sunMask = float(groundToSkyT >= 1.f);

  return mix(u_groundColor, skyGradient, groundToSkyT) + sun * sunMask;
}

void main() {
  vec3 col = getEnvironmentLight(normalize(v_viewVec));

  FragColor = vec4(col, 1.f);
}

