#version 460

out vec2 v_tex;

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

void main() {
  v_tex = vertices[gl_VertexID] * 0.5f + 0.5f;
  gl_Position = vec4(vertices[gl_VertexID], 0.f, 1.f);
}

