#include "game.h"
#include "window.h"
#include "gui.h"
#include <SDL.h>
#include "timing.h"
#include "timers.h"
#include "logger.h"

#define BALLOON_EXPLOSION_SCALE 2.5f
#define SHIP_EXPLOSION_SCALE 3.5f

namespace game
{
    game::PauseMenuActions arcadeModeGameLoop()
    {
        Window &window = Window::getInstance();
        Gui &gui = Gui::getInstance();

        // Initially generate world
        std::random_device rd;
        unsigned int randSeed = rd();
        infworld::worldseed permutations = infworld::makePermutations(randSeed, 8);
        infworld::ChunkTable chunktables[MAX_LOD];
        game::generateChunks(permutations, chunktables, RANGE);
        infworld::DecorationTable decorations = infworld::DecorationTable(14, CHUNK_SZ);
        decorations.genDecorations(permutations);
        gfx::generateDecorationOffsets(decorations);

        std::minstd_rand0 lcg(randSeed);

        // Gameobjects
        gameobjects::Player player(glm::vec3(0.0f, HEIGHT * SCALE * 0.5f, 0.0f));
        std::vector<gameobjects::Bullet> bullets;
        std::vector<gameobjects::Bullet> enemyBullets;
        std::vector<gameobjects::Enemy> balloons;
        std::vector<gameobjects::Enemy> ships;
        std::vector<gameobjects::Enemy> planes;
        std::vector<gameobjects::Explosion> explosions;
        std::vector<gameobjects::Props> barrels;

        float totalTime = 0.0f;
        float dt = 0.0f;
        unsigned int score = 0; // Player score
        bool draw_debug_gui = false;
        bool draw_rain = true;
        game::GameState gameState = game::RUNNING;

        TimerManager &timers = TimerManager::getInstance();
        timers.addTimer("spawn_balloon", 0.0f, 50.0f);
        timers.addTimer("spawn_ship", 0.0f, 150.0f);
        timers.addTimer("spawn_plane", 0.0f, 100.0f);
        // timers.addTimer("spawn_barrel", 0.0f, 70.0f);

        window.getCamera().updateCamera(player);

        std::vector<std::string> skyboxes = {"rainskybox", "nightskybox", "skybox"};
        std::string skybox = skyboxes.at(0);

        while (!window.shouldClose() && window.isRunnning())
        {
            double startTime = getTime();
            window.pollEvents();

            gui.newFrame();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // Draw terrain
            gfx::displayTerrain(chunktables, MAX_LOD, LOD_SCALE);
            // Display trees
            gfx::displayDecorations(decorations, totalTime);
            // Rain
            if (draw_rain)
                gfx::displayRain(totalTime);
            // Display plane
            if (!player.crashed)
                gfx::displayPlayerPlane(totalTime, player.transform, player.getPlayerObj());
            // Display balloons
            gfx::displayBalloons(balloons);
            // Display enemy ships
            gfx::displayShips(ships);
            // Display enemy planes
            gfx::displayPlanes(totalTime, planes);
            // Display barrels
            gfx::displayBarrels(barrels);
            // Display bullets
            gfx::displayBullets(bullets);
            gfx::displayBullets(enemyBullets);
            // Display water
            gfx::displayWater(totalTime);
            // Draw skybox
            gfx::displaySkybox(skybox);
            // Fog
            gfx::displayFog();
         
            if (gameState == game::RUNNING) {
                timers.update(dt);

                // Draw HUD Backgorunds
                gfx::displayCrosshair(player.transform);
                gfx::displayMiniMapBackground(totalTime);
                gfx::displayAttitude(player.transform.rotation.x, player.transform.rotation.z);
                gfx::displaySpeed(player.speed);
                gfx::displayFuel(player.fuel, totalTime);
                gfx::displayEnemyMarkers(balloons, player.transform);
                gfx::displayEnemyMarkers(ships, player.transform);
                gfx::displayEnemyMarkers(planes, player.transform);
                // gfx::displayPropMarkers(barrels, player.transform);
                gfx::displayExplosions(explosions);

                totalTime += dt;
                bool justcrashed = player.crashed;
                player.checkIfCrashed(dt, permutations);
                justcrashed = player.crashed ^ justcrashed;
                // Update explosions
                if (justcrashed) {
                    explosions.push_back(gameobjects::Explosion(player.transform.position));
                    // SNDSRC->playid("explosion", player.transform.position);
                    WARN("Plane Crashed");
                    gameState = game::DEAD;
                }
                player.update(dt);
                if (player.fuel <= 20.0f) {
                    gfx::displayDanger(totalTime);
                    // TODO: also need to add a sound to indicate the danger.
                }
                updateExplosions(explosions, player.transform.position, dt);

                // Shoot bullets
                KeyState leftbutton = window.getButtonState(SDL_BUTTON_LEFT);
                KeyState spacebar = window.getKeyState(SDLK_SPACE);
                if (player.shoottimer <= 0.0f && (window.keyIsHeld(spacebar) || window.keyIsHeld(leftbutton)) && !player.crashed){
                    // SNDSRC->playid("shoot", player.transform.position);
                    player.resetShootTimer();
                    bullets.push_back(gameobjects::Bullet(player, glm::vec3(-8.5f, -0.75f, 8.5f)));
                    bullets.push_back(gameobjects::Bullet(player, glm::vec3(8.5f, -0.75f, 8.5f)));
                }
                // Update bullets
                game::checkBulletDist(bullets, player);
                game::updateBullets(bullets, dt);
                game::checkForBulletTerrainCollision(bullets, permutations);
                checkForHit(bullets, balloons, 24.0f);
                checkForHit(bullets, ships, 32.0f);
                checkForHit(bullets, planes, 12.0f);
                // checkForHit(bullets, ufos, 14.0f);
                // Update Enemy Bullets
                game::checkBulletDist(enemyBullets, player);
                game::updateBullets(enemyBullets, dt);
                game::checkForBulletTerrainCollision(enemyBullets, permutations);
                game::checkForHit(enemyBullets, player, 10.0f);

                // Spawn Balloons
                if (timers.getTimer("spawn_balloon")){
                    INFO("BALLOONN");
                    spawnBalloons(player, balloons, lcg, permutations);
                }
                // Update Balloons
                for (auto &balloon : balloons)
                    balloon.updateBalloon(dt);
                // Destroy any enemies that are too far away or have run out of health
                destroyEnemies(player, balloons, explosions, BALLOON_EXPLOSION_SCALE, 24.0f, score);

                // Spawn Ships
                if (timers.getTimer("spawn_ship"))
                    spawnShips(player, ships, lcg, permutations);
                // Update Ships
                for (auto &ship : ships)
                    ship.updateShip(dt, player, enemyBullets);
                // Destroy any enemies that are too far away or have run out of health
                destroyEnemies(player, ships, explosions, SHIP_EXPLOSION_SCALE, 50.0f, score);

                // Spawn Planes
                if (timers.getTimer("spawn_plane"))
                    spawnPlanes(player, planes, lcg, permutations, totalTime);
                // Update Planes
                for (auto &plane : planes)
                    plane.updatePlane(dt, player, enemyBullets, permutations);
                // Destroy any enemies that are too far away or have run out of health
                destroyEnemies(player, planes, explosions, SHIP_EXPLOSION_SCALE, 50.0f, score);

                gui.drawHUD();

                window.getCamera().updateCamera(player, dt);

                // to make the terrain infinite
                game::generateNewChunks(permutations, chunktables, decorations);

                gui.dItems.playerPosition = player.transform.position;
                gui.dItems.cameraPosition = window.getCamera().position;
                gui.dItems.shipCount = ships.size();
                gui.dItems.balloonCount = balloons.size();

                // Update HUD data
                gui.hudItems.health = player.health;
                gui.hudItems.speed = player.speed;
                gui.hudItems.bulletCount = bullets.size();
                gui.hudItems.altitude = player.transform.position.y;
                gui.hudItems.score = score;
                gui.hudItems.crashed = player.crashed;
                gui.hudItems.fuel = player.fuel;
                gui.hudItems.elapsedTime = getTime();

            }
            else if (gameState == game::DEAD) {
                // Death should not be handled instantly, otherwise the player won't see the explosion
                // TRACE("Player Deathtimer: %.2f", player.deathtimer);
                if (player.deathtimer > 2.0f) {
                    switch (gui.drawDeathMenu())
                    {
                    case DEATH_EXIT:
                        window.setIsRunning(false);
                        break;
                    case DEATH_EXIT_TO_MAINMENU:
                        return game::EXIT_TO_MAINMENU;
                        break;
                    case TRY_AGAIN:
                        // Reset player
                        player = gameobjects::Player(glm::vec3(0.0f, HEIGHT * SCALE * 0.5f, 0.0f));
                        // Clear bullets and enemies
                        bullets.clear();
                        enemyBullets.clear();
                        balloons.clear();
                        ships.clear();
                        planes.clear();
                        explosions.clear();
                        barrels.clear();
                        gameState = game::RUNNING;
                        break;
                    default:
                        break;
                    }
                }else {
                    player.update(dt);
                    updateExplosions(explosions, player.transform.position, dt);
                    gfx::displayExplosions(explosions);
                }
            }
            else if (gameState == game::PAUSED) {
                // Pause should be handled instantly
                switch (game::PauseMenuActions action = gui.drawPauseMenu()) {
                case EXIT:
                    window.setIsRunning(false);
                    break;
                case EXIT_TO_MAINMENU:
                    return action;
                    break;
                case RESUME:
                    gameState = game::RUNNING;
                    break;
                case NONE:
                    break;
                default:
                    break;
                }
            }
            if (window.getKeyState(SDLK_m) == JUST_PRESSED) window.getCamera().shakeCamera(2.0f, 0.5f);
            if (window.getKeyState(SDLK_r) == JUST_PRESSED) draw_rain = !draw_rain;

            if (window.getKeyState(SDLK_TAB) == JUST_PRESSED)
                draw_debug_gui = !draw_debug_gui;
            if (window.getKeyState(SDLK_ESCAPE) == JUST_PRESSED){
                if (gameState == game::RUNNING)
                    gameState = game::PAUSED;
                else if (gameState == game::PAUSED)
                    gameState = game::RUNNING;
            }

            if (window.getKeyState(SDLK_p) == JUST_PRESSED)
            {
                static int i = 0;
                skybox = skyboxes.at(i);
                i = (i + 1) % skyboxes.size();
                // TRACE("selected skybox index = %d", i);
            }

            if (draw_debug_gui)
            {
                gui.drawUI();
            }

            gui.render();

            window.swapBuffers();
            window.updateKeyStates();
            dt = getTime() - startTime;
            timers.reset();
        }

        return NONE;
    }
}
