#version 460

out vec2 texCoord;

const vec2 vertices[] = vec2[](
  vec2(-1, -1),
  vec2(-1,  1),
  vec2( 1,  1),
  vec2( 1,  1),
  vec2( 1, -1),
  vec2(-1, -1)
);

void main() {
  texCoord = vertices[gl_VertexID] * 0.5f + 0.5f;
  gl_Position = vec4(vertices[gl_VertexID], 0.f, 1.f);
}

