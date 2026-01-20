#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "window.h"
#include "infworld.h"
#include "game.h"
#include "gfx.h"
#include "camera.h"
#include <iostream>
#include "gui.h"

int main() {
    
    Window& window = Window::getInstance();
    Gui& gui = Gui::getInstance();

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

        // gui.newFrame();
        game::GameMode mode = game::mainMenu();


        switch(mode){
            case game::SETTINGS:

                break;
            case game::ARCADE:
                game::arcadeModeGameLoop();
                break;
            default:
                break;
        }
        // gui.render();
    }

    return 0;
}
