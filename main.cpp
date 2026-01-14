#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "window.h"
#include "gui.h"
#include "infworld.h"
#include "game.h"
#include "gfx.h"
#include "camera.h"
#include <iostream>

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

    //Initially generate world
    std::random_device rd;
    int randSeed = rd();
    infworld::worldseed permutations = infworld::makePermutations(randSeed, 9);
    infworld::ChunkTable chunktables[MAX_LOD];
    game::generateChunks(permutations, chunktables, RANGE);
    infworld::DecorationTable decorations = infworld::DecorationTable(14, CHUNK_SZ);
    decorations.genDecorations(permutations);
    gfx::generateDecorationOffsets(decorations);

    //Gameobjects
		gameobjects::Player player(glm::vec3(0.0f, HEIGHT * SCALE * 0.5f, 0.0f));

  	float totalTime = 0.0f;
  	float dt = 0.0f;
    unsigned int chunksPerSecond = 0; //Number of chunks drawn per second	
    bool draw_debug_gui = true;

    game::updateCamera(player);
    
    while (!window.shouldClose() && window.isRunnning()) {
        float startTime = glfwGetTime();
        window.pollEvents();

        gui.newFrame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Draw terrain
        unsigned int drawCount = gfx::displayTerrain(chunktables, MAX_LOD, LOD_SCALE);
        chunksPerSecond += drawCount;
        //Display trees
        gfx::displayDecorations(decorations, totalTime);
        //Display plane
        if(!player.crashed)
           gfx::displayPlayerPlane(totalTime, player.transform);
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


        game::updateCamera(player, dt);
        // to make the terrain infinite
        game::generateNewChunks(permutations, chunktables, decorations);

        totalTime += dt;
        bool justcrashed = player.crashed;
				player.checkIfCrashed(dt, permutations);
				justcrashed = player.crashed ^ justcrashed;
				//Update explosions
				if(justcrashed) {
					// explosions.push_back(gobjs::Explosion(player.transform.position));
					// SNDSRC->playid("explosion", player.transform.position);	
					printf("Just crashed \n");
				}
        player.update(dt);
        
        if (window.getKeyState(GLFW_KEY_TAB) == JUST_PRESSED) draw_debug_gui = !draw_debug_gui;

        if (draw_debug_gui){
          gui.drawUI();
        }
        gui.render();

        window.swapBuffers();
        window.updateKeyStates();
        dt = glfwGetTime() - startTime;
    }

    return 0;
}
