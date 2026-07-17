#version 460 core

layout(location = 0) in vec3 a_pos;

out vec3 v_worldPos;
out vec3 v_viewVec;
out vec2 v_uv;

uniform mat4 u_camPV;
uniform vec3 u_camPos;
uniform float u_worldSize;

layout(binding = 0) uniform sampler2D u_texDisplacement;

void main() {
  vec2 uv = a_pos.xz * 0.5f + 0.5f;
  vec3 basePos = vec3(uv.x * u_worldSize, a_pos.y, uv.y * u_worldSize);
  vec3 displacement = texture(u_texDisplacement, uv).rgb;
  vec4 worldPos = vec4(basePos + displacement, 1.f);

  v_worldPos = worldPos.xyz;
  v_viewVec = u_camPos - v_worldPos.xyz;
  v_uv = uv;

  gl_Position = u_camPV * worldPos;
}

