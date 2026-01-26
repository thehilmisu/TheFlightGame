#include "game.h"
#include "window.h"
#include "gui.h"


namespace game {

  game::PauseMenuActions devModeGameLoop() {
      
    Window& window = Window::getInstance();
    Gui& gui = Gui::getInstance();

    //Initially generate world
    std::random_device rd;
    int randSeed = rd();
    infworld::worldseed permutations = infworld::makePermutations(randSeed, 9);
    infworld::ChunkTable chunktables[MAX_LOD];
    game::generateChunks(permutations, chunktables, RANGE);
    infworld::DecorationTable decorations = infworld::DecorationTable(14, CHUNK_SZ);
    decorations.genDecorations(permutations);
    gfx::generateDecorationOffsets(decorations);

    std::minstd_rand0 lcg;
		lcg.seed(randSeed);
		
    //Gameobjects
		gameobjects::Player player(glm::vec3(0.0f, HEIGHT * SCALE * 0.5f, 0.0f));
		std::vector<gameobjects::Bullet> bullets;
		std::vector<gameobjects::Enemy> balloons;
		std::vector<gameobjects::Explosion> explosions;
		std::vector<gameobjects::Props> barrels;

  	float totalTime = 0.0f;
  	float dt = 0.0f;
    unsigned int chunksPerSecond = 0; //Number of chunks drawn per second
    unsigned int score = 0; //Player score
    bool draw_debug_gui = false;
    bool paused = false;

    TimerManager timers;
    timers.addTimer("spawn_balloon", 0.0f, 20.0f);
    timers.addTimer("spawn_barrel", 0.0f, 20.0f);

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
           gfx::displayPlayerPlane(totalTime, player.transform, player.getPlayerObj());
        //Display balloons
        gfx::displayBalloons(balloons);
        //Display barrels
        gfx::displayBarrels(barrels);
        //Display bullets
        gfx::displayBullets(bullets);
        //Display water
        gfx::displayWater(totalTime);
        //Draw skybox
        gfx::displaySkybox();

        if (!paused) {

          timers.update(dt);
          
          //Draw HUD Backgorunds
          gfx::displayCrosshair(player.transform);
          gfx::displayMiniMapBackground();
          gfx::displayEnemyMarkers(balloons, player.transform);
          // gfx::displayPropMarkers(barrels, player.transform);
          gfx::displayExplosions(explosions);
        
          game::updateCamera(player, dt);
          // to make the terrain infinite
          game::generateNewChunks(permutations, chunktables, decorations);

          totalTime += dt;
          bool justcrashed = player.crashed;
  				player.checkIfCrashed(dt, permutations);
  				justcrashed = player.crashed ^ justcrashed;
  				//Update explosions
  				if(justcrashed) {
  					explosions.push_back(gameobjects::Explosion(player.transform.position));
  					// SNDSRC->playid("explosion", player.transform.position);	
  					printf("Just crashed \n");
  				}
          player.update(dt);
          updateExplosions(explosions, player.transform.position, dt);
          gui.dItems.playerPosition = player.transform.position;
          gui.dItems.cameraPosition = window.getCamera().position;
          gui.dItems.bulletCount = bullets.size();

          // Update HUD data
          gui.hudItems.health = player.health;
          gui.hudItems.speed = player.speed;
          gui.hudItems.bulletCount = bullets.size();
          gui.hudItems.altitude = player.transform.position.y;
          gui.hudItems.score = score;
          gui.hudItems.crashed = player.crashed;
          gui.hudItems.fuel = player.fuel;

          //Shoot bullets
  				KeyState leftbutton = window.getButtonState(GLFW_MOUSE_BUTTON_LEFT);
  				KeyState spacebar = window.getKeyState(GLFW_KEY_SPACE);
  				if(player.shoottimer <= 0.0f && 
  				   (window.keyIsHeld(spacebar) || window.keyIsHeld(leftbutton)) &&
  				   !player.crashed) {
  					// SNDSRC->playid("shoot", player.transform.position);
  					player.resetShootTimer();
  					bullets.push_back(gameobjects::Bullet(player, glm::vec3(-8.5f, -0.75f, 8.5f)));
  					bullets.push_back(gameobjects::Bullet(player, glm::vec3(8.5f, -0.75f, 8.5f)));
  				}
  				//Update bullets
  				game::checkBulletDist(bullets, player);
  				game::updateBullets(bullets, dt);
  				game::checkForBulletTerrainCollision(bullets, permutations);
  				checkForHit(bullets, balloons, 24.0f);
  				// checkForHit(bullets, blimps, 32.0f);
  				// checkForHit(bullets, ufos, 14.0f);
  				// checkForHit(bullets, planes, 12.0f);

          // Spawn Balloons
          if(timers.getTimer("spawn_balloon")) spawnBalloons(player, balloons, lcg, permutations);
          // Update Balloons
          for( auto &balloon : balloons) balloon.updateBalloon(dt);
          //Destroy any enemies that are too far away or have run out of health
  				destroyEnemies(player, balloons, explosions, 1.0f, 24.0f, score);

          // Spawn Barrels
          if(timers.getTimer("spawn_barrel")) spawnBarrels(player, barrels, lcg, permutations);
          // Update Barrels
          for( auto &barrel : barrels) barrel.updateBarrel(dt);
          //Destroy any enemies that are too far away or have run out of health
  				destroyProps(player, barrels, explosions, 1.0f, 24.0f, score);
  				
          gui.drawHUD();
        
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
        
        if (window.getKeyState(GLFW_KEY_TAB) == JUST_PRESSED) draw_debug_gui = !draw_debug_gui;
        if (window.getKeyState(GLFW_KEY_ESCAPE) == JUST_PRESSED) paused = !paused;

        if (draw_debug_gui){
          gui.drawUI();
        }

        gui.render();

        window.swapBuffers();
        window.updateKeyStates();
        dt = glfwGetTime() - startTime;
    }

    return NONE;
  }

}
