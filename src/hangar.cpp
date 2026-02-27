#include "game.h"
#include "window.h"
#include "camera.h"
#include "gui.h"
#include <SDL.h>
#include "timing.h"
#include "logger.h"

namespace game {

  game::PauseMenuActions hangarGameLoop() {
      
    Window& window = Window::getInstance();
    Gui& gui = Gui::getInstance();

    //Gameobjects
		gameobjects::Player player(glm::vec3(40.0f, 9.0f, 50.0f));
  	float totalTime = 0.0f;
  	float dt = 0.0f;
    bool paused = false;

    float mRotation = 0.0f;
    float rotationDirection = 1.0f;

    // Position camera inside the hangar, looking at the plane
    Camera& cam = window.getCamera();
    cam.position = glm::vec3(40.0f, 28.0f, 100.0f);
    cam.yaw = 0.0f;    // look in -Z toward the plane
    cam.pitch = 0.18f; // slight downward tilt (~15 degrees)

    
    unsigned int current_selected = player.getCurrentIndex();

    while (!window.shouldClose() && window.isRunnning()) {
        float startTime = getTime();
        window.pollEvents();

        gui.newFrame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gfx::displayHangar3(45.0f);
        //Display plane
        gfx::displayPlayerPlane(totalTime, player.transform, player.getPlayerObj());
        game::MainMenuActions ac = gui.drawPlaneSelectionUI();
        if (ac == CHANGE_PLANE_MINUS) {
          current_selected = (current_selected - 1) % 2;
          player.setPlayerObj(current_selected);          
        }
        else if (ac == CHANGE_PLANE_PLUS) {
          current_selected = (current_selected + 1) % 2;
          player.setPlayerObj(current_selected);
        }
        
        if (!paused) {
          // window.getCamera().updateCamera(player, dt);
          totalTime += dt;

          // Just rotate the player
          // TODO: add full rotation or maybe drag with mouse
          mRotation += 0.1f * dt * rotationDirection;
          if (mRotation > glm::radians(60.0f) || mRotation < glm::radians(-60.0f))
            rotationDirection *= -1.0f;

          player.transform.rotation.y = mRotation;


        }
        //paused
        else{
          game::PauseMenuActions action = gui.drawPauseMenu();
          switch (action) {
            case EXIT:
              window.setIsRunning(false);
              break;
            case EXIT_TO_MAINMENU:
              return action;
              break;
            case RESUME:
              paused = false;
              break;
            case NONE:
              break;
            default:
            break;
           }
        }
        
        if (window.getKeyState(SDLK_ESCAPE) == JUST_PRESSED) paused = !paused;

        gui.render();

        window.swapBuffers();
        window.updateKeyStates();
        dt = getTime() - startTime;
    }

    return NONE;
  }
}
