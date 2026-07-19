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
water::Tessendorf* gui::waterPtrTessendorf = nullptr;
Sun* gui::sunPtr = nullptr;
TextureCubemap* gui::skyboxTexPtr = nullptr;

u16 gui::fps = 1;

namespace {

void RenderTexture(ImTextureRef tex, ImVec2 size = ImVec2(0, 0)) {
  bool useCustomSize = size.x || size.y;

  vec2 winSize = global::getWinSize();
  float winLongestPart = glm::max(winSize.x, winSize.y);
  ImVec2 imgSize = useCustomSize ? size : ImVec2(vec2(winLongestPart * 0.125f));
  ImVec2 imgUV0 = vec2(0.f, 1.f);
  ImVec2 imgUV1 = vec2(1.f, 0.f);

  Image(tex, imgSize, imgUV0, imgUV1);
}

// Undo swizzle?
[[maybe_unused]]
void ApplySwizzle(const Texture& tex, const std::array<GLint, 4>& swizzle) {
  tex.bind();
  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle.data());
  tex.unbind();
}

struct LoaderWidget {
  std::string bufLoad;
  std::string bufSave;

  LoaderWidget(const std::string& name) {
    bufLoad.reserve(256);
    bufSave.reserve(256);
    bufLoad = bufSave =  name;
  }

  bool render(auto& cfg) {
    bool u = false;

    ImGui::InputText(".json##0", bufLoad.data(), 256 * sizeof(char)); SameLine();
    if (Button("Load") && bufLoad[0]) {
      water::loadPreset(cfg, std::format("{}.json", bufLoad));
      u = true;
    }

    ImGui::InputText(".json##1", bufSave.data(), 256 * sizeof(char)); SameLine();
    if (Button("Save") && bufSave[0])
      water::savePreset(cfg, std::format("{}.json", bufSave));

    return u;
  }
};

} // namespace

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

        if (global::waterAlgorithm == Tessendorf)
          water::texResolution = glm::min(water::texResolution, 1024);

        u = true;
      }
    }

    Text("Mesh resolution (%d)", water::meshResolution);
    SameLine();
    {
      bool um = false;

      if (ArrowButton("##left##2", ImGuiDir_Left)) {
        water::meshResolution >>= 1;
        water::meshResolution = glm::max(water::meshResolution, 128);
        um = true;
      }
      SameLine();

      if (ArrowButton("##right##2", ImGuiDir_Right)) {
        water::meshResolution <<= 1;
        water::meshResolution = glm::min(water::meshResolution, 512);
        um = true;
      }

      if (um)
        water::updateMesh();
    }

    if (u) {
      water::updateWorkGroups();
      waterPtrSOSA->rebuild();
      waterPtrGerstner->rebuild();
      waterPtrTessendorf->rebuild = true;
    }

    if (RadioButton("Sum of sines approximation", global::waterAlgorithm == SOSA)) global::waterAlgorithm = SOSA;
    if (RadioButton("Gerstner##1", global::waterAlgorithm == Gerstner)) global::waterAlgorithm = Gerstner;
    if (RadioButton("Tessendorf", global::waterAlgorithm == Tessendorf)) global::waterAlgorithm = Tessendorf;

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

        SeparatorText("Texture");
        RenderTexture(waterPtrSOSA->texNormheight.getId());

        SeparatorText("Load/Save");
        static LoaderWidget lw("sosa0");
        lw.render(*waterPtrSOSA);

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
        static LoaderWidget lw("gerstner0");
        lw.render(*waterPtrGerstner);

        SeparatorText("Displacement and Normal textures");
        RenderTexture(waterPtrGerstner->texDisplacement.getId()); SameLine();
        RenderTexture(waterPtrGerstner->texNormal.getId());

        break;
      }
      case Tessendorf:
      {
        assert(waterPtrFFT);
        SliderFloat("Mesh scale", &waterPtrTessendorf->worldSize, 1.f, 5000.f);
        SliderFloat("Foam strength", &waterPtrTessendorf->foamSharpness, 0.f, 3.f);

        SeparatorText("Noise settings");
        {
          bool u = false;
          u |= DragFloat("Seed XIr", &waterPtrTessendorf->seed1);
          u |= DragFloat("Seed XIi", &waterPtrTessendorf->seed2);

          if (u) {
            waterPtrTessendorf->generateNoise();
            waterPtrTessendorf->generateInitialSpectrum();
          }
        }
        SeparatorText("Gaussian noise / Butterfly");
        RenderTexture(waterPtrTessendorf->texNoise.getId()); SameLine();
        {
          // ugh....
          vec2 winSize = global::getWinSize();
          float winLongestPart = glm::max(winSize.x, winSize.y);
          ImVec2 texSize{(float)waterPtrTessendorf->logSize, std::min((float)waterPtrTessendorf->size, winLongestPart * 0.125f)};
          RenderTexture(waterPtrTessendorf->texButterfly.getId(), texSize);
        }

        SeparatorText("General spectrum settings");
        {
          bool u = false;

          u |= SliderFloat("G", &waterPtrTessendorf->g, 0.f, 100.f);
          u |= SliderFloat("Depth", &waterPtrTessendorf->depth, 0.f, 100.f);
          u |= SliderFloat("Lambda", &waterPtrTessendorf->lambda, 0.f, 1.f);
          u |= SliderFloat("Length scale", &waterPtrTessendorf->lengthScale, 0.f, 1000.f);

          if (TreeNode("Local spectrum settings")) {
            auto& settings = waterPtrTessendorf->local;
            u |= SliderFloat("Scale", &settings.scale, 0.f, 1.f);
            u |= SliderFloat("Wind speed", &settings.windSpeed, 0.f, 100.f);
            u |= SliderAngle("Wind direction", &settings.windDir, 0.f);
            u |= SliderFloat("Fetch", &settings.fetch, 0.f, 1e6f);
            u |= SliderFloat("Spread blend", &settings.spreadBlend, 0.f, 1.f);
            u |= SliderFloat("Swell", &settings.swell, 0.f, 1.f);
            u |= SliderFloat("Peak enhancement", &settings.peakEnhancemnt, 0.f, 100.f);
            u |= SliderFloat("Short waves fade", &settings.shortWavesFade, 0.f, 2.f);

            TreePop();
          }

          if (TreeNode("Swell spectrum settings")) {
            auto& settings = waterPtrTessendorf->swell;
            u |= SliderFloat("Scale##2", &settings.scale, 0.f, 1.f);
            u |= SliderFloat("Wind speed##2", &settings.windSpeed, 0.f, 100.f);
            u |= SliderAngle("Wind direction##2", &settings.windDir, 0.f);
            u |= SliderFloat("Fetch##2", &settings.fetch, 0.f, 1e6f);
            u |= SliderFloat("Spread blend##2", &settings.spreadBlend, 0.f, 1.f);
            u |= SliderFloat("Swell##2", &settings.swell, 0.f, 1.f);
            u |= SliderFloat("Peak enhancement##2", &settings.peakEnhancemnt, 0.f, 100.f);
            u |= SliderFloat("Short waves fade##2", &settings.shortWavesFade, 0.f, 2.f);

            TreePop();
          }

          if (u)
            waterPtrTessendorf->generateInitialSpectrum();
        }

        SeparatorText("Precomputed Data / Conjugated Spectrum");
        RenderTexture(waterPtrTessendorf->texPrecomputedData.getId()); SameLine();
        RenderTexture(waterPtrTessendorf->texInitialSpectrum.getId());

        SeparatorText("Displacement / Derivatives / Turbulence");
        RenderTexture(waterPtrTessendorf->texDisplacement.getId()); SameLine();
        RenderTexture(waterPtrTessendorf->texDerivatives.getId()); SameLine();

        ApplySwizzle(waterPtrTessendorf->texTurbulence, {GL_RED, GL_RED, GL_RED, GL_ONE});
        RenderTexture(waterPtrTessendorf->texTurbulence.getId());

        SeparatorText("Load/Save");
        static LoaderWidget lw("tessendorf0");
        if (lw.render(*waterPtrTessendorf))
          waterPtrTessendorf->rebuild = true;

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

  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 posBR = viewport->WorkPos + viewport->WorkSize;

  ImGui::SetNextWindowPos(posBR, ImGuiCond_Always, {1.f, 1.f});
  // ImGui::SetNextWindowCollapsed(infoCollapsed);

  global::profiler.renderTasks(400, 200, 200, 0);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void gui::shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

