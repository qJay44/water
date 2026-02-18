#version 460

out vec3 v_viewVec;

uniform mat4 u_camInv;
uniform vec3 u_camPos;

const vec2 vertices[] = vec2[](
  vec2(-1, -1),
  vec2(-1,  1),
  vec2( 1,  1),
  vec2( 1,  1),
  vec2( 1, -1),
  vec2(-1, -1)
);

vec3 getViewVec(vec2 uv) {
  vec2 ndc = uv * 2.f - 1.f;
  vec4 clipPos = vec4(ndc, -1.f, 1.f);
  vec4 worldPos = u_camInv * clipPos;
  worldPos /= worldPos.w;

  return worldPos.xyz - u_camPos;
}

void main() {
  vec2 vertex = vertices[gl_VertexID];
  v_viewVec = getViewVec(vertex * 0.5f + 0.5f);
  gl_Position = vec4(vertex, 0.f, 1.f);
}

