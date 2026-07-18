#include "glm/trigonometric.hpp"
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
  #include <direct.h>
  #define CHDIR(p) _chdir(p);
#else
  #include <unistd.h>
  #define CHDIR(p) chdir(p);
#endif

#include "engine/Camera.hpp"
#include "engine/InputsHandler.hpp"
#include "engine/Shader.hpp"
#include "engine/gui/gui.hpp"
#include "engine/mesh/MeshElements.hpp"
#include "engine/mesh/meshes.hpp"
#include "global.hpp"
#include "other/Sun.hpp"
#include "utils/clrp.hpp"
#include "water/ConfigManager.hpp"
#include "water/Tessendorf.hpp"
#include "water/Gerstner.hpp"
#include "water/SOSA.hpp"
#include "water/general.hpp"

using global::window;

void GLAPIENTRY MessageCallback(
  GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  const void* userParam
) {
  static const clrp::clrp_t clrpError{clrp::ATTRIBUTE::BOLD, clrp::FG::RED};
  static const clrp::clrp_t clrpWarning{clrp::ATTRIBUTE::BOLD, clrp::FG::YELLOW};

  clrp::clrp_t clrpFinal = clrpError;
  bool stop = true;
  switch (source) {
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      return; // Handled by the Shader class itself
  }

  // Suppress annoyoing SIMD32 callback
  if (type == GL_DEBUG_TYPE_PERFORMANCE) {
    clrpFinal = clrpWarning;
    stop = false;
  }

  fprintf(
    stderr, "GL CALLBACK: %s source = 0x%x, id = 0x%x type = 0x%x, severity = 0x%x, message = %s\n",
    (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), source, id, type, severity, clrp::format(message, clrpError).c_str()
  );

  if (stop)
    exit(EXIT_FAILURE);
}

int main() {
  // Assuming the executable is launching from its own directory
  CHDIR("../../..");

  // GLFW init
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

  // Window init
  window = glfwCreateWindow(1600, 900, "MyProgram", NULL, NULL);
  ivec2 winSize = global::getWinSize();
  dvec2 winCenter = dvec2(winSize) / 2.;

  if (!window) {
    printf("Failed to create GFLW window\n");
    glfwTerminate();
    return EXIT_FAILURE;
  }
  glfwMakeContextCurrent(window);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + 2 * !global::guiFocused);
  glfwSetCursorPos(window, winCenter.x, winCenter.y);

  // GLAD init
  if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    return EXIT_FAILURE;
  }

  glViewport(0, 0, winSize.x, winSize.y);
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(MessageCallback, 0);

  gui::init();

  // ===== Shaders ============================================== //

  Shader::setDirectoryLocation("res/shaders");

  Shader shaderAxis("axis.vert", "axis.frag");
  Shader shaderWaterSOSA("water/sosa.vert", "water/sosa.frag");
  Shader shaderWaterGerstner("water/gerstner.vert", "water/gerstner.frag");
  Shader shaderWaterTessendorf("water/tessendorf.vert", "water/tessendorf.frag");
  Shader shaderSkybox("skybox.vert", "skybox.frag");

  // ===== Cameras ============================================== //

  Camera camera({-400.f, 400.f, 400.f}, 0, glm::radians(-25.f));
  camera.setFarPlane(10000.f);
  camera.setSpeedDefault(100.f);

  // ===== Inputs Handler ======================================= //

  InputsHandler::mousePos = global::getWinCenter();
  glfwSetScrollCallback(window, InputsHandler::scrollCallback);
  glfwSetKeyCallback(window, InputsHandler::keyCallback);
  glfwSetCursorPosCallback(window, InputsHandler::cursorPosCallback);

  // ===== Sun (light) ========================================== //

  Sun sun{};
  sun.pitch = PI_6 * 0.5f;
  sun.updateDir();

  // ===== Water ================================================ //

  water::init();
  water::SOSA waterSOSA{};
  water::loadPreset(waterSOSA, "sosa0.json");

  water::Gerstner waterGerstner{};
  water::loadPreset(waterGerstner, "gerstner0.json");

  water::Tessendorf waterTessendorf{};
  water::loadPreset(waterTessendorf, "tessendorf0.json");
  waterTessendorf.updateInitials();

  // ===== Other ================================================ //

  auto meshSkybox = MeshElements::loadFromOBJ("res/obj/Cube.obj");
  auto axis = meshes::axis();

  axis.scale(1e4f);

  auto texSkybox = TextureCubemap::loadFromImage("res/tex/Cubemaps/Cubemap_Sky_04-512x512.png", {.target = GL_TEXTURE_CUBE_MAP});

  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  gui::camPtr = &camera;
  gui::waterPtrSOSA = &waterSOSA;
  gui::waterPtrGerstner = &waterGerstner;
  gui::waterPtrTessendorf = &waterTessendorf;
  gui::sunPtr = &sun;
  gui::skyboxTexPtr = &texSkybox;

  global::waterAlgorithm = global::WaterAlgorithm::Tessendorf;

  // Render loop
  while (!glfwWindowShouldClose(window)) {
    static double titleTimer = glfwGetTime();
    static double prevTime = titleTimer;
    static double currTime = prevTime;

    constexpr double fpsLimit = 1. / 90.;
    currTime = glfwGetTime();
    global::dt = currTime - prevTime;

    // FPS cap
    if (global::dt < fpsLimit) continue;
    else prevTime = currTime;

    global::time += global::dt;
    winCenter = global::getWinCenter();

    if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
      InputsHandler::process(camera);
      camera.update();
    }

    if (!global::guiFocused)
      glfwSetCursorPos(global::window, winCenter.x, winCenter.y);

    // Update fps every 0.3 seconds
    if (currTime - titleTimer >= 0.3) {
      gui::fps = static_cast<u16>(1.f / global::dt);
      titleTimer = currTime;
    }

    // ===== Updates ============================================== //

    using enum global::WaterAlgorithm;

    sun.setUniforms(shaderWaterSOSA);
    sun.setUniforms(shaderWaterGerstner);
    sun.setUniforms(shaderWaterTessendorf);

    switch (global::waterAlgorithm) {
      case SOSA:
        waterSOSA.update();
        break;
      case Gerstner:
        waterGerstner.update();
        break;
      case Tessendorf:
        waterTessendorf.update();
        break;
    }

    // ===== Main framebuffer ===================================== //

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE + !global::wireframeMode);
    glDepthFunc(GL_LEQUAL);

    texSkybox.bind(0);
    meshSkybox.draw(&camera, shaderSkybox);

    glDepthFunc(GL_LESS);

    switch (global::waterAlgorithm) {
      case SOSA:
        shaderWaterSOSA.setUniform1f("u_worldSize", waterSOSA.worldSize);

        waterSOSA.texNormheight.bind(0);
        texSkybox.bind(1);

        water::mesh.draw(&camera, shaderWaterSOSA);
        break;
      case Gerstner:
        shaderWaterGerstner.setUniform1f("u_worldSize", waterGerstner.worldSize);

        waterGerstner.texDisplacement.bind(0);
        waterGerstner.texNormal.bind(1);
        texSkybox.bind(2);

        water::mesh.draw(&camera, shaderWaterGerstner);
        break;
      case Tessendorf:
        texSkybox.bind(3);
        waterTessendorf.draw(&water::mesh, &camera, shaderWaterTessendorf);
        break;
    }

    if (global::drawWorldAxis)
      axis.draw(&camera, shaderAxis);

    // ============================================================ //

    gui::draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  gui::shutdown();
  glfwTerminate();

  return 0;
}

