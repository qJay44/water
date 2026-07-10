#version 460 core

layout(location = 0) in vec3 a_pos;

out vec3 v_worldPos;
out vec3 v_viewVec;
out vec2 v_uv;

uniform mat4 u_camPV;
uniform mat4 u_model;
uniform vec3 u_camPos;

layout(binding = 0) uniform sampler2D u_texNormheight;

void main() {
  vec2 uv = a_pos.xz * 0.5f + 0.5f;
  vec3 pos = a_pos;
  pos.y = texture(u_texNormheight, uv).a;

  vec4 worldPos = u_model * vec4(pos, 1.f);

  v_worldPos = worldPos.xyz;
  v_viewVec = u_camPos - v_worldPos.xyz;
  v_uv = uv;

  gl_Position = u_camPV * worldPos;
}

