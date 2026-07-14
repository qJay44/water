#pragma once

#include "../../water/SOSA.hpp"
#include "../../water/Gerstner.hpp"
#include "../../water/FFT.hpp"
#include "../../other/Sun.hpp"
#include "../texture/TextureCubemap.hpp"
#include "../Camera.hpp"

struct gui {
  static Camera* camPtr;
  static water::SOSA* waterPtrSOSA;
  static water::Gerstner* waterPtrGerstner;
  static water::FFT* waterPtrFFT;
  static Sun* sunPtr;
  static TextureCubemap* skyboxTexPtr;
  static u16 fps;

  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
  static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

  static void init();
  static void toggle();
  static void draw();
  static void shutdown();
};

