#include "gui.hpp"

#include <format>

#include "imgui.h"
// #include "implot.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "glm/gtc/type_ptr.hpp"

using namespace ImGui;

static bool collapsed = true;

Camera* gui::camPtr = nullptr;
Water* gui::waterPtr = nullptr;
Sun* gui::sunPtr = nullptr;
Fog* gui::fogPtr = nullptr;
Texture* gui::skyboxTexPtr = nullptr;

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
  // ImPlot::CreateContext();
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

  Begin("Settings");

  ImGui::Text("FPS: %d / %f.5 ms", fps, global::dt);

  // ===== Spectate camera =============================================================================== //

  if (!camPtr) error("The spectate camera is not linked to gui");
  if (CollapsingHeader("Spectate camera")) {
    SliderFloat("Near##2", &camPtr->nearPlane, 0.01f, 1.f);
    SliderFloat("Far##2", &camPtr->farPlane,  10.f, 1000.f);
    SliderFloat("Speed##2", &camPtr->speedDefault, 1.f, 50.f);
    SliderFloat("FOV##2", &camPtr->fov, 45.f, 179.f);
    DragFloat("Yaw##2", &camPtr->yaw);
    DragFloat("Pitch##2", &camPtr->pitch);
    DragFloat3("Position", glm::value_ptr(camPtr->position));

    if (TreeNode("Flags")) {
      CheckboxFlags("Right", &camPtr->flags, CameraFlags_DrawRight);
      CheckboxFlags("Up", &camPtr->flags, CameraFlags_DrawUp);
      CheckboxFlags("Forward", &camPtr->flags, CameraFlags_DrawForward);

      TreePop();
    }
  }

  // ===== Water ========================================================================================= //

  if (!waterPtr) error("The water is not linked to gui");
  if (CollapsingHeader("Water")) {
    SeparatorText("Plane mesh");
    if (SliderInt("Resolution", &waterPtr->resolution, 2, 500))
      waterPtr->rebuild();

    if (SliderFloat("Scale", &waterPtr->scale, 0.f, 10000.f))
      waterPtr->setScale({waterPtr->scale * 0.5f, 0.f, waterPtr->scale * 0.5f});

    if (TreeNode("Info")) {
      float sizeBytes = sizeof(VertexPT) * waterPtr->verticesCount;
      Text("Vertices: %zu (%.2f MB)", waterPtr->verticesCount, sizeBytes * 0.001f);
      Text("Indices: %zu", waterPtr->indicesCount);
      TreePop();
    }

    SeparatorText("Wave");
    SliderFloat("Wavelength", &waterPtr->wavelength, 0.f, 100.f);
    SliderFloat("Speed", &waterPtr->speed, 0.f, 100.f);
    SliderFloat("Amplitude", &waterPtr->amplitude, 0.f, 100.f);
    SliderInt("Waves", &waterPtr->waves, 1, 32);

    SeparatorText("Sum of waves");
    SliderFloat("Persistence", &waterPtr->persistence, 0.f, 1.f);
    SliderFloat("Lacunarity", &waterPtr->lacunarity, 1.f, 10.f);
    SliderFloat("Speed mutiplier", &waterPtr->speedMul, 0.f, 2.f);
    SliderFloat("Drag mutiplier", &waterPtr->dragMul, 0.f, 2.f);

    SeparatorText("Load/Save");

    static char bufLoad[256]{"waves0"};
    static char bufSave[256]{"waves0"};

    InputText(".json##0", bufLoad, sizeof(bufLoad)); SameLine();
    if (Button("Load") && bufLoad[0])
      waterPtr->loadPreset(std::format("{}.json", bufLoad));

    InputText(".json##1", bufSave, sizeof(bufSave)); SameLine();
    if (Button("Save") && bufSave[0])
      waterPtr->savePreset(std::format("{}.json", bufSave));
  }

  // ===== Fog =========================================================================================== //

  if (!fogPtr) error("The fog is not linked to gui");
  if (CollapsingHeader("Fog")) {
    SliderFloat("Thinness", &fogPtr->thinness, 0.f, 2.f);
    SliderFloat("Start", &fogPtr->start, 0.f, 10000.f);
    ColorEdit3("Color", glm::value_ptr(fogPtr->color));
  }

  // ===== Light ========================================================================================= //

  if (!sunPtr) error("The light is not linked to gui");
  if (CollapsingHeader("Light")) {
    bool upd = false;

    DragFloat("Focus", &sunPtr->focus, 1.f, 0.f);
    DragFloat("Intensity", &sunPtr->intensity, 1.f, 0.f);
    upd |= DragFloat("Yaw##3", &sunPtr->yaw, PI_2 * 0.01f);
    upd |= DragFloat("Pitch##3", &sunPtr->pitch, PI_2 * 0.01f);
    ColorEdit3("Color", glm::value_ptr(sunPtr->color));

    if (upd)
      sunPtr->updateDir();
  };

  // ===== Other ========================================================================================= //

  if (CollapsingHeader("Other")) {
    Checkbox("Show global axis", &global::drawGlobalAxis);
    if (Button("New skybox")) {
      TextureDescriptor desc = skyboxTexPtr->getDescriptor();
      int num = rand() % 25 + 1;

      delete skyboxTexPtr;
      skyboxTexPtr = new Texture(std::format("res/tex/Cubemaps/Cubemap_Sky_{:02}-512x512.png", num), desc);
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
  // ImPlot::DestroyContext();
  ImGui::DestroyContext();
}

