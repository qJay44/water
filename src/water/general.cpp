#include "general.hpp"

#include "../engine/mesh/meshes.hpp"

namespace water {

int texResolution = 1024; // NOTE: Keep this in power of 2
int meshResolution = 512;

MeshElements mesh;
int numWorkGroups;

void init() {
  updateWorkGroups();
  updateMesh();
}

void updateWorkGroups() {
  numWorkGroups = texResolution / 8;
}

void updateMesh() {
  mesh = meshes::plane(meshResolution);
}

} // namespace water

