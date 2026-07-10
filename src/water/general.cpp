#include "general.hpp"

#include "SOSA.hpp"
#include "Gerstner.hpp"

namespace water {

int texResolution = 1024;
int meshResolution = 512;
MeshElements mesh;

int numWorkGroups = texResolution / 16;

void init() {
  update();
}

void update() {
  numWorkGroups = (texResolution + 15) / 16;
  mesh = meshes::plane(meshResolution);
}

} // namespace water

