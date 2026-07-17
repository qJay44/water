#pragma once

#include "../engine/mesh/MeshElements.hpp"

namespace water {

extern int texResolution;
extern int meshResolution;
extern MeshElements mesh;

extern int numWorkGroups;

void init();
void updateWorkGroups();
void updateMesh();

} // namespace water

