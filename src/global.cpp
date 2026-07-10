#include "global.hpp"

namespace global {

GLFWwindow* window = nullptr;
WaterAlgorithm waterAlgorithm = WaterAlgorithm::SOSA;

float dt = 0.f;
float time = 0.f;

bool wireframeMode = false;
bool guiFocused = false;
bool drawGrid = false;
bool drawWorldAxis = false;

}// global

