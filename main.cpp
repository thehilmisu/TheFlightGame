#ifdef __ANDROID__
  #include <GLES2/gl3.h>
#else
#include <glad/glad.h>
#include <SDL.h>
#endif

#include "camera.h"
#include "game.h"
#include "gfx.h"
#include "gui.h"
#include "imgui.h"
#include "infworld.h"
#include "window.h"
#include "logger.h"
#include "assetloader.h"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  Window &window = Window::getInstance();
  Gui &gui = Gui::getInstance();

  window.initMousePos();
  window.getCamera().pitch = -0.5f;

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  int fbWidth, fbHeight;
  SDL_GL_GetDrawableSize(window.getSDLWindow(), &fbWidth, &fbHeight);
  glViewport(0, 0, fbWidth, fbHeight);
  window.updatePerspectiveMat(FOVY, ZNEAR, ZFAR, fbWidth, fbHeight);

  //game::loadAssets();
  //game::initUniforms();

  AssetLoader &assetLoader = AssetLoader::getInstance();
  assetLoader.loadAssets("assets.bin");
  auto shadercode = assetLoader.getAssetData("textured-frag.glsl");
  std::string shaderStr(shadercode.begin(), shadercode.end());
  std::cout << "Loaded shader code " << std::endl;
  std::cout <<  shaderStr.c_str() << std::endl;

  // while (!window.shouldClose() && window.isRunnning()) {

  //   game::MainMenuActions action = game::mainMenu();
  //   INFO("Action selected from main menu : %d ", action);

  //   switch (action) {
  //   case game::OPTIONS:
  //     if (game::devModeGameLoop() == game::EXIT_TO_MAINMENU) {
  //       gui.render();
  //       action = game::mainMenu();
  //     }
  //     break;
  //   case game::START_GAME:
  //     if (game::arcadeModeGameLoop() == game::EXIT_TO_MAINMENU) {
  //         gui.render();
  //         action = game::mainMenu();
  //       }
  //     break;
  //   case game::EXIT_GAME:
  //     game::exitGame();
  //     break;
  //   default:
  //     break;
  //   }
  // }

  return 0;
}
