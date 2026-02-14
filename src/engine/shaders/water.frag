#version 460 core

out vec4 FragColor;

in vec4 worldPos;
in vec3 normal;

uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform vec3 u_camPos;

vec3 directionalLight() {
  vec3 lightDistVec = u_lightPos - worldPos.xyz;
  float lightDist = length(lightDistVec);

  vec3 lightDir = normalize(lightDistVec);
  vec3 viewDir = normalize(u_camPos - worldPos.xyz);
  vec3 reflectDir = reflect(-lightDir, normal);

  float diffuse = max(dot(normal, lightDir), 0.f);
  float ambient = 0.01f;
  float specularLight = 0.5f;
  float specAmount = pow(max(dot(viewDir, reflectDir), 0.f), 8);
  float specular = specAmount * specularLight;

  float lightAmount = (diffuse + specular) + ambient;

  return u_lightColor * lightAmount;
}

void main() {
  vec3 col = vec3(0.2f, 0.f, 0.68f);
  col *= directionalLight();

  FragColor = vec4(col, 1.f);
}

