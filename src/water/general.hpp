#pragma once

#include "../engine/mesh/meshes.hpp"

namespace water {

extern int texResolution;
extern int meshResolution;
extern MeshElements mesh;

extern int numWorkGroups;

void init();
void update();

} // namespace water

