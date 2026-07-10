#version 460

in vec3 v_viewVec;

out vec4 FragColor;

uniform vec3 u_camPos;
uniform vec3 u_sunColor;
uniform vec3 u_sunDir;
uniform float u_sunFocus;
uniform float u_sunIntensity;

vec3 getSun(vec3 viewDir) {
  float sun = pow(max(0, dot(viewDir, -u_sunDir)), u_sunFocus) * u_sunIntensity;
  return u_sunColor * sun;
}

void main() {
  vec3 viewDir = normalize(v_viewVec);
  vec3 col = getSun(viewDir);

  FragColor = vec4(col, 1.f);
}

