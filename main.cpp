#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "window.h"
#include "gui.h"
#include "infworld.h"
#include "game.h"
#include "gfx.h"
#include "camera.h"
#include <iostream>

// glm::vec3 getCameraFollowPos(const game::Transform &playertransform)
// {
// 	glm::vec3 offset = playertransform.rotate(glm::vec3(0.0f, 28.0f, -60.0f));
// 	return playertransform.position + offset;
// }

// void updateCamera(game::Transform transform)
// {
// 	Camera& cam = Window::getInstance().getCamera();
// 	//Update camera
// 	cam.position = game::getCameraFollowPos(transform);
// 	cam.yaw = -(transform.rotation.y + glm::radians(180.0f));
// 	cam.pitch = transform.rotation.x;
// }

int main() {
    
    Window& window = Window::getInstance();
    Gui& gui = Gui::getInstance();

    game::loadAssets();
    game::initUniforms();

    //Initially generate world
    std::random_device rd;
    int randSeed = rd();
    infworld::worldseed permutations = infworld::makePermutations(randSeed, 9);
    infworld::ChunkTable chunktables[MAX_LOD];
    game::generateChunks(permutations, chunktables, RANGE);
    infworld::DecorationTable decorations = infworld::DecorationTable(14, CHUNK_SZ);
    decorations.genDecorations(permutations);
    gfx::generateDecorationOffsets(decorations);

    // game::update_camera(
    //     glm::vec3(0.0f, HEIGHT * SCALE * 0.5f, 0.0f)
    // );
  	float totalTime = 0.0f;
    unsigned int chunksPerSecond = 0; //Number of chunks drawn per second	
    bool draw_debug_gui = true;
    
    while (!window.shouldClose() && window.isRunnning()) {
        window.pollEvents();

        gui.newFrame();

        window.updatePerspectiveMat(FOVY, ZNEAR, ZFAR, window.getWidth(), window.getHeight());

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Draw terrain
        unsigned int drawCount = gfx::displayTerrain(chunktables, MAX_LOD, LOD_SCALE);
        chunksPerSecond += drawCount;
        //Display trees
        gfx::displayDecorations(decorations, totalTime);
        //Display plane
        //if(!player.crashed)
        //    gfx::displayPlayerPlane(totalTime, player.transform);
        //Display water
        gfx::displayWater(totalTime);
        //Draw skybox
        gfx::displaySkybox();
        //Display explosions
        //gfx::displayExplosions(explosions);
        //User Interface
        //gui::displayFPSCounter(fps);
        // if(player.crashed && !paused && player.deathtimer > 2.5f)
        //     gui::displayDeathScreen(0);

        if (window.getKeyState(GLFW_KEY_TAB) == JUST_PRESSED) draw_debug_gui = !draw_debug_gui;
        window.getCamera().rotateCamera(window.getMouseDX(), window.getMouseDY(), 1.0f);

        if (draw_debug_gui){
          gui.drawUI();
        }
        gui.render();

        window.swapBuffers();
        window.updateKeyStates();
    }

    return 0;
}
