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
		gameobjects::Player player(glm::vec3(-71.0, 5.0f, 32.0f));
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

    
    unsigned int current_selected = player.getCurrentIndex();
    game::PauseMenuActions action = NONE;

    while (!window.shouldClose() && window.isRunnning()) {
        float startTime = getTime();
        window.pollEvents();

        gui.newFrame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gfx::displayHangar(5.0f);
        //Display plane
        gfx::displayPlayerPlane(totalTime, player.transform, player.getPlayerObj());
        game::MainMenuActions ac = gui.drawPlaneSelectionUI(player.getPlayerObj());
        if (ac == CHANGE_PLANE_MINUS) {
          current_selected = (current_selected - 1) % 5;
          player.setPlayerObj(current_selected);          
        }
        else if (ac == CHANGE_PLANE_PLUS) {
          current_selected = (current_selected + 1) % 5;
          player.setPlayerObj(current_selected);
        }
        else if (ac == PLANE_SELECTED) {
          player.setPlayerObj(current_selected);
          return EXIT_TO_MAINMENU;
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
               
#if 0 //Debugging purposes
        if (window.getKeyState(SDLK_w) == HELD) cam.position.z -= 1.0f;
        if (window.getKeyState(SDLK_a) == HELD) cam.position.x -= 1.0f;
        if (window.getKeyState(SDLK_s) == HELD) cam.position.z += 1.0f;
        if (window.getKeyState(SDLK_d) == HELD) cam.position.x += 1.0f;
        if (window.getKeyState(SDLK_q) == HELD) cam.position.y += 1.0f;
        if (window.getKeyState(SDLK_e) == HELD) cam.position.y -= 1.0f;

        cam.rotateCamera(window.getMouseDX(), window.getMouseDY(), 0.5f);
        // TRACE("x : %f, y : %f, dx : %f, dy : %f", window.getMouseX(), window.getMouseY(), window.getMouseDX(), window.getMouseDY());
        TRACE("x : %f, y : %f, z : %f", cam.position.x, cam.position.y, cam.position.z);
        TRACE("pitch : %f, yaw : %f", cam.pitch, cam.yaw);
        if (window.getKeyState(SDLK_w) == HELD) player.transform.position.z -= 1.0f;
        if (window.getKeyState(SDLK_a) == HELD) player.transform.position.x -= 1.0f;
        if (window.getKeyState(SDLK_s) == HELD) player.transform.position.z += 1.0f;
        if (window.getKeyState(SDLK_d) == HELD) player.transform.position.x += 1.0f;
        if (window.getKeyState(SDLK_q) == HELD) player.transform.position.y += 1.0f;
        if (window.getKeyState(SDLK_e) == HELD) player.transform.position.y -= 1.0f;

        TRACE("x : %f, y : %f, z : %f", player.transform.position.x, player.transform.position.y, player.transform.position.z);
#endif
        gui.render();

        window.swapBuffers();
        window.updateKeyStates();
        dt = getTime() - startTime;
    }

    return NONE;
  }
}
