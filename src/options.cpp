#include "game.h"
#include "window.h"
#include "camera.h"
#include "gui.h"
#include <SDL.h>
#include "timing.h"
#include "logger.h"

namespace game {

  game::MainMenuActions optionsGameLoop() {
      
    Window& window = Window::getInstance();
    Gui& gui = Gui::getInstance();

  	float totalTime = 0.0f;
  	float dt = 0.0f;
    bool paused = false;

    float mRotation = 0.0f;
    float rotationDirection = 1.0f;

    // Position camera inside the hangar, looking at the plane
    Camera& cam = window.getCamera();
    cam.position = glm::vec3(-102.f, 11.0f, 34.0f);
    cam.yaw = -4.7f;    
    cam.pitch = 0.07f; 

    
    game::MainMenuActions action = NONE_SELECTED;

    while (!window.shouldClose() && window.isRunnning()) {
        float startTime = getTime();
        window.pollEvents();

        gui.newFrame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gfx::displayHangar(5.0f);

        action = gui.drawOptionsMenu();
        if (action == BACK_TO_MAINMENU) {
          return BACK_TO_MAINMENU;
        }
        
        if (!paused) {
          totalTime += dt;

        }

        gui.render();

        window.swapBuffers();
        window.updateKeyStates();
        dt = getTime() - startTime;
    }

    return action;
  }
}
