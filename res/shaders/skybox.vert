#version 460

layout (location = 0) in vec3 a_pos;

out vec3 v_uv;

uniform mat4 u_camProj;
uniform mat4 u_camView;

void main() {
  v_uv = a_pos;
  mat4 skyboxView = mat4(mat3(u_camView));
  vec4 clipSpace = u_camProj * skyboxView * vec4(a_pos, 1.f);
  gl_Position = clipSpace.xyww;
}

