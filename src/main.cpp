#include "engine/lighting/Sun.hpp"
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
  #include <direct.h>
  #define CHDIR(p) _chdir(p);
#else
  #include <unistd.h>
  #define CHDIR(p) chdir(p);
#endif

#include "engine/gui/gui.hpp"
#include "engine/Camera.hpp"
#include "engine/Shader.hpp"
#include "engine/InputsHandler.hpp"
#include "engine/mesh/meshes.hpp"
#include "engine/FBO.hpp"
#include "engine/Fog.hpp"
#include "engine/Water.hpp"
#include "engine/texture/TextureDescriptor.hpp"
#include "utils/clrp.hpp"

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
  switch (source) {
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      return; // Handled by the Shader class itself
    case GL_DEBUG_SOURCE_API:
      return; // "SIMD32 shader inefficient", skipping since occurs only on my laptop
  }

  clrp::clrp_t clrpError;
  clrpError.attr = clrp::ATTRIBUTE::BOLD;
  clrpError.fg = clrp::FG::RED;
  fprintf(
    stderr, "GL CALLBACK: %s source = 0x%x, id = 0x%x type = 0x%x, severity = 0x%x, message = %s\n",
    (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), source, id, type, severity, clrp::format(message, clrpError).c_str()
  );
  exit(1);
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

  Shader::setDirectoryLocation("src/engine/shaders");

  Shader lightShader("light.vert", "light.frag");
  Shader linesShader("lines.vert", "lines.frag");
  Shader waterShader("water.vert", "water.frag");
  Shader postprocessShader("postprocess.vert", "postprocess.frag");
  Shader sunShader("sun.vert", "sun.frag");
  Shader skyboxShader("skybox.vert", "skybox.frag");

  // ===== Cameras ============================================== //

  Camera cameraSpectate({85.f, 77.f, 76.f}, -PI_2);
  cameraSpectate.setFarPlane(5000.f);
  cameraSpectate.setSpeedDefault(100.f);

  // ===== Inputs Handler ======================================= //

  InputsHandler::mousePos = global::getWinCenter();
  glfwSetScrollCallback(window, InputsHandler::scrollCallback);
  glfwSetKeyCallback(window, InputsHandler::keyCallback);
  glfwSetCursorPosCallback(window, InputsHandler::cursorPosCallback);

  // ===== Framebuffers ========================================= //

  TextureDescriptor fboTexDesc;
  fboTexDesc.uniformName = "u_sceneColorTex";
  fboTexDesc.unit = 0;
  fboTexDesc.minFilter = GL_NEAREST;
  fboTexDesc.magFilter = GL_NEAREST;
  fboTexDesc.genMipMap = false;

  Texture sceneColorTex({winSize}, fboTexDesc);

  fboTexDesc.uniformName = "u_sceneDepthTex";
  fboTexDesc.unit = 1;
  fboTexDesc.internalFormat = GL_DEPTH_COMPONENT24;
  fboTexDesc.format = GL_DEPTH_COMPONENT;
  Texture sceneDepthTex({winSize}, fboTexDesc);

  FBO fboScene;
  fboScene.attach2D(GL_COLOR_ATTACHMENT0, sceneColorTex);
  fboScene.attach2D(GL_DEPTH_ATTACHMENT, sceneDepthTex);

  // ============================================================ //

  Sun sun(800.f, 2.f, 0.f, PI_6 * 0.5f);
  sun.updateDir();

  Water water(100, 5000.f);
  water.loadPreset("waves0.json");

  Mesh axis = meshes::axis();
  axis.scale(1e4f);

  Fog fog{vec3(1.f), 10.f, 100.f};

  TextureDescriptor cubemapTexDesc;
  cubemapTexDesc.uniformName = "u_skyboxTex";
  cubemapTexDesc.unit = 2;
  cubemapTexDesc.target = GL_TEXTURE_CUBE_MAP;
  cubemapTexDesc.minFilter = GL_LINEAR;
  cubemapTexDesc.magFilter = GL_LINEAR;
  cubemapTexDesc.wrapS = GL_CLAMP_TO_EDGE;
  cubemapTexDesc.wrapT = GL_CLAMP_TO_EDGE;
  cubemapTexDesc.wrapR = GL_CLAMP_TO_EDGE;
  cubemapTexDesc.genMipMap = false;
  cubemapTexDesc.cubemapLoad = TextureCubemapLoad_FromCubemapImage;
  Texture* skyboxTex = new Texture("res/tex/Cubemaps/Cubemap_Sky_04-512x512.png", cubemapTexDesc);

  Mesh skyboxCube = Mesh::loadObj("res/obj/Cube.obj");

  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  gui::camPtr = &cameraSpectate;
  gui::waterPtr = &water;
  gui::sunPtr = &sun;
  gui::fogPtr = &fog;
  gui::skyboxTexPtr = skyboxTex;

  Camera* cam = &cameraSpectate;

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

    if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
      InputsHandler::process(cameraSpectate);
      cameraSpectate.update();
    } else
      glfwSetCursorPos(window, winCenter.x, winCenter.y);

    // Update fps every 0.3 seconds
    if (currTime - titleTimer >= 0.3) {
      gui::fps = static_cast<u16>(1.f / global::dt);
      titleTimer = currTime;
    }

    water.setUniforms(waterShader);
    fog.setUniforms(postprocessShader);
    sun.setUniforms(waterShader);
    waterShader.setUniformMatrix4f("u_camInv", cam->getProjViewInv());
    postprocessShader.setUniformMatrix4f("u_camInv", cam->getProjViewInv());

    // ===== Scene framebuffer ==================================== //

    fboScene.bind();
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    skyboxTex->bind();

    glDepthFunc(GL_LEQUAL);
    skyboxCube.draw(cam, skyboxShader);
    glDepthFunc(GL_LESS);

    sun.draw(cam, sunShader);

    glEnable(GL_CULL_FACE);
    water.draw(cam, waterShader);

    skyboxTex->unbind();

    // ===== Main framebuffer ===================================== //

    FBO::unbind();
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    sceneDepthTex.bind();
    sceneColorTex.bind();
    Mesh::screenDraw(cam, postprocessShader);
    sceneDepthTex.unbind();
    sceneColorTex.unbind();

    if (global::drawGlobalAxis)
      axis.draw(cam, linesShader);

    // ============================================================ //

    gui::draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  gui::shutdown();
  glfwTerminate();

  return 0;
}

