#include "game.h"
#include "gui.h"
#include "window.h"
#include <SDL.h>
#include "timing.h"

namespace gobjs = gameobjects;

namespace game
{

  void exitGame()
  {
    Window &window = Window::getInstance();
    window.setIsRunning(false);
  }

  game::MainMenuActions mainMenu()
  {
    Window &window = Window::getInstance();
    Gui &gui = Gui::getInstance();

    window.setCursorInputMode(CURSOR_NORMAL);

    window.getCamera().pitch = 0.0f;
    window.getCamera().yaw = 0.0f;
    window.getCamera().position = glm::vec3(0.0f);

    gobjs::Player player(glm::vec3(14.0f, -8.0f, -40.0f));
    gobjs::Plane enemy(glm::vec3(0.0f, 50.0f, -100.0f), 40.0f);
    gobjs::Balloon balloon(glm::vec3(100.0f, 50.0f, -150.0f));
    gobjs::Balloon balloon1(glm::vec3(-100.0f, 20.0f, -200.0f));
    enemy.transform.rotation.x = glm::radians(40.0f);
    std::vector<gobjs::Plane> enemies = {enemy};
    std::vector<gobjs::Balloon> balloons = {balloon, balloon1};

    float dt = 0.0f;
    float totalTime = 0.0f;
    std::string skybox = "nightskybox";
    float mRotation = enemy.transform.rotation.y;
    float rotationDirection = 1.0f;
    while (!window.shouldClose()) {

      auto start = static_cast<float>(getTime());

      gui.newFrame();

      // Update perspective matrix
      window.updatePerspectiveMat(FOVY, ZNEAR, ZFAR, window.getWidth(),
                                  window.getHeight());

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      gfx::displayPlayerPlane(totalTime, player.transform, player.getPlayerObj());
      gfx::displayPlanes(totalTime, enemies);
      gfx::displayBalloons(balloons);

      // Display skybox
      gfx::displaySkybox(skybox);

      game::MainMenuActions selected = gui.drawMainMenu();

      
      mRotation += 0.1f * dt * rotationDirection;
      if (mRotation > glm::radians(60.0f) || mRotation < glm::radians(-60.0f))
        rotationDirection *= -1.0f;
      
      for (auto &e : enemies) {
        e.transform.rotation.y = -mRotation;
        //e.update(0.0f, {});
      }
   
      gui.render();

      window.updateKeyStates();
      glEnable(GL_CULL_FACE);
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);
      window.swapBuffers();
      window.pollEvents();
      gfx::outputErrors();
      totalTime += dt;

      unsigned int current_selected = player.getCurrentIndex();
      switch (selected)
      {
      case HANGAR:
        return selected;
      case START_GAME:
        return selected;
      case CREDITS:
        break;
      case CHANGE_PLANE_MINUS:
      {
        current_selected = (current_selected - 1) % 2;
        player.setPlayerObj(current_selected);
        skybox = "skybox";
        break;
      }
      case CHANGE_PLANE_PLUS:
      {
        current_selected = (current_selected + 1) % 2;
        player.setPlayerObj(current_selected);
        skybox = "nightskybox";
        break;
      }
      case EXIT_GAME:
      {
        return selected;
      }
      default:
        break;
      }

      dt = static_cast<float>(getTime()) - start;
    }

    return NONE_SELECTED;
  }
} // namespace game
