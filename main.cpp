#include "camera.h"
#include "game.h"
#include "gfx.h"
#include "gui.h"
#include "infworld.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>

int main() {

  Window &window = Window::getInstance();

  window.initMousePos();
  window.getCamera().pitch = -0.5f;

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  int fbWidth, fbHeight;
  glfwGetFramebufferSize(glfwGetCurrentContext(), &fbWidth, &fbHeight);
  glViewport(0, 0, fbWidth, fbHeight);
  window.updatePerspectiveMat(FOVY, ZNEAR, ZFAR, fbWidth, fbHeight);

  game::loadAssets();
  game::initUniforms();

  while (!window.shouldClose() && window.isRunnning()) {

    game::MainMenuActions action = game::mainMenu();
    std::cout << "action : " << action << std::endl;
    switch (action) {
    case game::OPTIONS:

      break;
    case game::START_GAME:
      game::arcadeModeGameLoop();
      break;
    case game::EXIT_GAME:
      game::exitGame();
      break;
    default:
      break;
    }
  }

  return 0;
}
