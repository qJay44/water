#include "gui.hpp"

#include <cassert>
#include <format>

#include "global.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "glm/gtc/type_ptr.hpp"

#include "../../water/ConfigManager.hpp"

using namespace ImGui;

static bool collapsed = true;

Camera* gui::camPtr = nullptr;
water::SOSA* gui::waterPtrSOSA = nullptr;
water::Gerstner* gui::waterPtrGerstner = nullptr;
water::FFT* gui::waterPtrFFT = nullptr;
Sun* gui::sunPtr = nullptr;
TextureCubemap* gui::skyboxTexPtr = nullptr;

u16 gui::fps = 1;

void gui::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}

void gui::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

void gui::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
  ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
}

void gui::init() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
  ImGui_ImplGlfw_InitForOpenGL(global::window, true);
  ImGui_ImplOpenGL3_Init();
}

void gui::toggle() { collapsed = !collapsed; }

void gui::draw() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  SetNextWindowCollapsed(collapsed);

  const vec2 winSize = global::getWinSize();
  const float winLongestPart = glm::max(winSize.x, winSize.y);
  const ImVec2 imgSize = vec2(winLongestPart * 0.125f);

  Begin("Settings");

  ImGui::Text("FPS: %d / %f.5 ms", fps, global::dt);

  // ===== Spectate camera =============================================================================== //

  assert(camPtr);
  if (CollapsingHeader("Spectate camera")) {
    SliderFloat("Near##1", &camPtr->nearPlane, 0.01f, 1.f);
    SliderFloat("Far##1", &camPtr->farPlane,  10.f, 1000.f);
    SliderFloat("Speed##1", &camPtr->speedDefault, 1.f, 50.f);
    SliderFloat("FOV##1", &camPtr->fov, 45.f, 179.f);
    SliderAngle("Yaw##1", &camPtr->yaw, -180.f, 180.f);
    SliderAngle("Pitch##1", &camPtr->pitch, -89.f, 89.f);
    DragFloat3("Position", glm::value_ptr(camPtr->position));
  }

  // ===== Water ========================================================================================= //

  if (CollapsingHeader("Water")) {
    using enum global::WaterAlgorithm;

    bool u = false;

    Text("Texture resolution (%d)", water::texResolution);
    SameLine();
    {
      if (ArrowButton("##left##1", ImGuiDir_Left)) {
        water::texResolution >>= 1;
        water::texResolution = glm::max(water::texResolution, 128);
        u = true;
      }
      SameLine();

      if (ArrowButton("##right##1", ImGuiDir_Right)) {
        water::texResolution <<= 1;
        water::texResolution = glm::min(water::texResolution, 8192);
        u = true;
      }
    }

    Text("Mesh resolution (%d)", water::meshResolution);
    SameLine();
    {
      if (ArrowButton("##left##2", ImGuiDir_Left)) {
        water::meshResolution >>= 1;
        water::meshResolution = glm::max(water::texResolution, 128);
        u = true;
      }
      SameLine();

      if (ArrowButton("##right##2", ImGuiDir_Right)) {
        water::meshResolution <<= 1;
        water::meshResolution = glm::min(water::meshResolution, 512);
        u = true;
      }
    }

    if (u) {
      water::update();
      waterPtrSOSA->rebuild();
      waterPtrGerstner->rebuild();
      waterPtrFFT->rebuild();
    }

    if (RadioButton("Sum of sines approximation", global::waterAlgorithm == SOSA)) global::waterAlgorithm = SOSA;
    if (RadioButton("Gerstner##1", global::waterAlgorithm == Gerstner)) global::waterAlgorithm = Gerstner;
    if (RadioButton("Fourier transform", global::waterAlgorithm == FFT)) global::waterAlgorithm = FFT;

    switch (global::waterAlgorithm) {
      case SOSA:
      {
        assert(waterPtrSOSA);

        SeparatorText("Config");
        SliderFloat("World size", &waterPtrSOSA->worldSize, 0.f, 1000.f);
        SliderFloat("Wavelength", &waterPtrSOSA->wavelength, 0.f, 100.f);
        SliderFloat("Speed", &waterPtrSOSA->speed, 0.f, 100.f);
        SliderFloat("Amplitude", &waterPtrSOSA->amplitude, 0.f, 100.f);
        SliderFloat("Persistence", &waterPtrSOSA->persistence, 0.f, 1.f);
        SliderFloat("Lacunarity", &waterPtrSOSA->lacunarity, 1.f, 10.f);
        SliderFloat("Speed mutiplier", &waterPtrSOSA->speedMul, 0.f, 2.f);
        SliderFloat("Drag mutiplier", &waterPtrSOSA->dragMul, 0.f, 2.f);
        SliderInt("Waves", &waterPtrSOSA->waves, 1, 32);

        SeparatorText("Load/Save");

        static char bufLoad[256]{"sosa0"};
        static char bufSave[256]{"sosa0"};

        InputText(".json##0", bufLoad, sizeof(bufLoad)); SameLine();
        if (Button("Load") && bufLoad[0])
          water::loadPreset(*waterPtrSOSA, std::format("{}.json", bufLoad));

        InputText(".json##1", bufSave, sizeof(bufSave)); SameLine();
        if (Button("Save") && bufSave[0])
          water::savePreset(*waterPtrSOSA, std::format("{}.json", bufSave));

        SeparatorText("Texture");
        Image(waterPtrSOSA->texNormheight.getId(), imgSize);
        break;
      }
      case Gerstner:
      {
        assert(waterPtrGerstner);

        SliderFloat("World size", &waterPtrGerstner->worldSize,  0.f, 1000.f);
        SliderFloat("Wavelength", &waterPtrGerstner->wavelength, 0.f, 100.f);
        SliderFloat("Amplitude",  &waterPtrGerstner->amplitude,  0.f, 100.f);
        SliderFloat("Frequency",  &waterPtrGerstner->frequency,  0.f, 100.f);
        SliderFloat("Phase",      &waterPtrGerstner->phase,      0.f, 100.f);
        SliderInt("Count (N)",    &waterPtrGerstner->N,          0  , 20);

        SliderFloat("Wavelength (step)", &waterPtrGerstner->wavelengthStep, 0.f, 10.f);
        SliderFloat("Amplitude (step)",  &waterPtrGerstner->amplitudeStep,  0.f, 1.f);
        SliderFloat("Frequency (step)",  &waterPtrGerstner->frequencyStep,  0.f, 10.f);
        SliderFloat("Phase (step)",      &waterPtrGerstner->phaseStep,      0.f, 10.f);
        SliderAngle("Angle (step)",      &waterPtrGerstner->angleStep);

        SeparatorText("Load/Save");

        static char bufLoad[256]{"gerstner0"};
        static char bufSave[256]{"gerstner0"};

        InputText(".json##0", bufLoad, sizeof(bufLoad)); SameLine();
        if (Button("Load") && bufLoad[0])
          water::loadPreset(*waterPtrGerstner, std::format("{}.json", bufLoad));

        InputText(".json##1", bufSave, sizeof(bufSave)); SameLine();
        if (Button("Save") && bufSave[0])
          water::savePreset(*waterPtrGerstner, std::format("{}.json", bufSave));

        SeparatorText("Displacement and Normal textures");
        Image(waterPtrGerstner->texDisplacement.getId(), imgSize); SameLine();
        Image(waterPtrGerstner->texNormal.getId(), imgSize);

        break;
      }
      case FFT:
      {
        assert(waterPtrFFT);

        SeparatorText("Gaussian noise");
        {
          bool u = false;
          u |= DragFloat("Seed XIr", &waterPtrFFT->seed1);
          u |= DragFloat("Seed XIi", &waterPtrFFT->seed2);

          if (u) {
            waterPtrFFT->generateNoise();
            waterPtrFFT->generateWaveHeightField();
          }
        }

        SeparatorText("The spectrum part");
        {
          bool u = false;
          u |= SliderFloat("Amplitude",  &waterPtrFFT->amplitude,  0.f, 100.f);
          u |= SliderFloat("Wind speed", &waterPtrFFT->windSpeed,  0.f, 100.f);
          u |= SliderFloat("G",          &waterPtrFFT->g,  0.f, 100.f);
          u |= SliderAngle("Wind angle", &waterPtrFFT->windAngle, 0.f, 360.f);

          if (u)
            waterPtrFFT->generateWaveHeightField();
        }

        SeparatorText("Gaussian noise / h0(k) with omega / h(k,t)");
        Image(waterPtrFFT->texNoise.getId(), imgSize); SameLine();
        Image(waterPtrFFT->texWaveHeightField.getId(), imgSize); SameLine();
        Image(waterPtrFFT->texWaveHeightFieldTimeEvolution.getId(), imgSize);

        SeparatorText("IFFTs: ChoppyX / Height field / ChoppyZ");
        SliderFloat("Choppiness", &waterPtrFFT->choppinessControl,  0.f, 100.f);
        Image(waterPtrFFT->texWaveHeightFieldTimeEvolution_IFFT.getId(), imgSize); SameLine();
        Image(waterPtrFFT->texWaveChoppyX_IFFT.getId(), imgSize); SameLine();
        Image(waterPtrFFT->texWaveChoppyZ_IFFT.getId(), imgSize);

        SeparatorText("Displacement");
        Image(waterPtrFFT->texDisplacement.getId(), imgSize);

        break;
      }
    }
  }

  // ===== Light ========================================================================================= //

  assert(sunPtr);
  if (CollapsingHeader("Light")) {
    bool upd = false;

    DragFloat("Focus", &sunPtr->focus, 1.f, 0.f);
    DragFloat("Intensity", &sunPtr->intensity, 1.f, 0.f);
    upd |= SliderAngle("Yaw##2", &sunPtr->yaw, -180.f, 180.f);
    upd |= SliderAngle("Pitch##2", &sunPtr->pitch, -90.f, 90.f);
    ColorEdit3("Color", glm::value_ptr(sunPtr->color));

    if (upd)
      sunPtr->updateDir();
  };

  // ===== Other ========================================================================================= //

  if (CollapsingHeader("Other")) {
    Checkbox("Show world axis", &global::drawWorldAxis);
    if (Button("New skybox")) {
      int num = rand() % 25 + 1;
      *skyboxTexPtr = TextureCubemap::loadFromImage(std::format("res/tex/Cubemaps/Cubemap_Sky_{:02}-512x512.png", num), {.target = GL_TEXTURE_CUBE_MAP});
    }
  }

  // ===================================================================================================== //

  End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void gui::shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

