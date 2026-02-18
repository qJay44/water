#pragma once

#include "../lighting/Sun.hpp"
#include "../Water.hpp"
#include "../Fog.hpp"

struct gui {
  static Camera* camPtr;
  static Water* waterPtr;
  static Sun* sunPtr;
  static Fog* fogPtr;
  static u16 fps;

  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
  static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

  static void init();
  static void toggle();
  static void draw();
  static void shutdown();
};

