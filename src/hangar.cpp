#include "game.h"
#include "window.h"
#include "gui.h"
#include <SDL.h>
#include "timing.h"


namespace game {

  game::PauseMenuActions hangarGameLoop() {
      
    Window& window = Window::getInstance();
    Gui& gui = Gui::getInstance();

    //Gameobjects
		gameobjects::Player player(glm::vec3(0.0f, HEIGHT * SCALE * 0.5f, 0.0f));
  	float totalTime = 0.0f;
  	float dt = 0.0f;
    bool paused = false;

    window.getCamera().updateCamera(player);
    
    while (!window.shouldClose() && window.isRunnning()) {
        float startTime = getTime();
        window.pollEvents();

        gui.newFrame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       //Display plane
        gfx::displayPlayerPlane(totalTime, player.transform, player.getPlayerObj());
        if (!paused) {
          window.getCamera().updateCamera(player, dt);
          totalTime += dt;
          // player.update(0.0f);
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
