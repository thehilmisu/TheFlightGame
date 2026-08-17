#include "game.h"
#include "window.h"
#include "gui.h"
#include <SDL.h>
#include "timing.h"
#include "timers.h"
#include "logger.h"
#include "audio.h"
#include "settings.h"

#define BALLOON_EXPLOSION_SCALE 2.5f
#define SHIP_EXPLOSION_SCALE 3.5f

namespace game {
    PauseMenuActions arcadeModeGameLoop() {
        Window &window = Window::getInstance();
        Gui &gui = Gui::getInstance();

        // Initially generate world
        std::random_device rd;
        unsigned int randSeed = rd();
        infworld::worldseed permutations = infworld::makePermutations(randSeed, 8);
        infworld::ChunkTable chunktables[MAX_LOD];
        generateChunks(permutations, chunktables, RANGE);
        infworld::DecorationTable decorations = infworld::DecorationTable(14, CHUNK_SZ);
        decorations.genDecorations(permutations);
        gfx::generateDecorationOffsets(decorations);


        std::minstd_rand0 lcg(randSeed);

        // Gameobjects
        gameobjects::Player player(glm::vec3(0.0f, HEIGHT * SCALE * 0.5f, 0.0f));
        std::vector<gameobjects::Bullet> bullets;
        std::vector<gameobjects::Rocket> rockets;
        std::vector<gameobjects::Bullet> enemyBullets;
        std::vector<gameobjects::Balloon> balloons;
        std::vector<gameobjects::Ship> ships;
        std::vector<gameobjects::Plane> planes;
        std::vector<gameobjects::Explosion> explosions;
        std::vector<gameobjects::Barrel> barrels;


        //Time survived in the current run. It drives the HUD clock, the shader
        //animations and the difficulty ramp, so a restart has to put it back to
        //zero along with everything else.
        float totalTime = 0.0f;
        float dt = 0.0f;
        unsigned int score = 0; // Player score
        bool newHighScore = false;
        game::TargetLock targetLock; // Missile lock, rebuilt every frame
        bool draw_debug_gui = false;
        bool draw_rain = true;
        bool free_look = false;
        game::GameState gameState = game::RUNNING;

        TimerManager &timers = TimerManager::getInstance();
        timers.addTimer("spawn_balloon", 0.0f, 50.0f);
        //Ships are capped at 3 and only spawn at sea, so a 150s interval left
        //most runs without a single one
        timers.addTimer("spawn_ship", 0.0f, 60.0f);
        timers.addTimer("spawn_plane", 0.0f, 100.0f);
        timers.addTimer("spawn_barrel", 0.0f, 70.0f);
        //addTimer only inserts when the name is new, so a second run through
        //this loop would inherit the previous run's countdowns without this
        timers.resetAll();

        window.getCamera().updateCamera(player);

        std::vector<std::string> skyboxes = {"rainskybox", "nightskybox", "skybox"};
        std::string skybox = skyboxes.at(0);

        while (!window.shouldClose() && window.isRunnning()) {
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
            // Display Rockets
            gfx::displayRockets(rockets);
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
                gfx::displayPlaneHealth(static_cast<float>(player.health), totalTime);
                gfx::displayRocketAmmo(player.rockets, MAX_ROCKETS);
                gfx::displayTargetLock(targetLock);
                // Minimap contacts, colour coded by threat
                gfx::displayEnemyMarkers(planes, player.transform, glm::vec3(1.0f, 0.15f, 0.05f));
                gfx::displayEnemyMarkers(ships, player.transform, glm::vec3(1.0f, 0.55f, 0.05f));
                gfx::displayEnemyMarkers(balloons, player.transform, glm::vec3(0.95f, 0.9f, 0.2f));
                gfx::displayExplosions(explosions);

                totalTime += dt;
                bool justcrashed = player.crashed;
                player.checkIfCrashed(dt, permutations);
                justcrashed = player.crashed ^ justcrashed;
                // Update explosions
                if (justcrashed) {
                    explosions.emplace_back(player.transform.position);
                    SNDSRC->playid("explosion", player.transform.position);
                    WARN("Plane Crashed");
                    gameState = game::DEAD;
                    //Record the run straight away - submitScore writes the file
                    //itself so an unclean exit cannot lose the result
                    newHighScore = SETTINGS.submitScore(score);
                }
                player.update(dt);
                if (player.fuel <= 20.0f) {
                    gfx::displayDanger(totalTime);
                    // TODO: also need to add a sound to indicate the danger.
                }
                updateExplosions(explosions, player.transform.position, dt);

                // Shoot bullets
                // KeyState leftbutton = window.getButtonState(SDL_BUTTON_LEFT);
                KeyState spacebar = window.getKeyState(SDLK_SPACE);
                if (player.shoottimer <= 0.0f && (window.keyIsHeld(spacebar) /*|| window.keyIsHeld(leftbutton)*/) && !
                    player.crashed) {
                    SNDSRC->playid("shoot", player.transform.position);
                    player.resetShootTimer();
                    bullets.emplace_back(player, glm::vec3(-8.5f, -0.75f, 8.5f));
                    bullets.emplace_back(player, glm::vec3(8.5f, -0.75f, 8.5f));
                }
                // Everything that needs to follow a specific enemy across
                // frames resolves it through here
                const game::TargetDirectory targets = {
                    .planes = &planes,
                    .ships = &ships,
                    .balloons = &balloons
                };
                game::updateTargetLock(targetLock, player, targets, dt);

                // Rockets. getKeyState is truthy for HELD as well as
                // JUST_PRESSED, so this has to test the transition explicitly
                // or holding the key launches a rocket every single frame.
                if (player.rockets > 0 &&
                    player.rockettimer <= 0.0f &&
                    window.getKeyState(SDLK_g) == JUST_PRESSED &&
                    !player.crashed) {
                    player.resetRocketTimer();
                    player.rockets--;
                    // A solid lock guides the rocket; firing without one is a
                    // dumb shot that flies wherever the nose was pointing
                    const unsigned int target = targetLock.locked ? targetLock.targetId : 0;
                    rockets.emplace_back(player, glm::vec3(-8.5f, -0.75f, 8.5f), target);
                    SNDSRC->playid("shoot", player.transform.position);
                }
                game::updateRockets(rockets, dt, targets);
                game::checkForRocketTerrainCollision(rockets, explosions, permutations);
                // Update bullets
                game::checkBulletDist(bullets, player);
                game::updateBullets(bullets, dt);
                game::checkForBulletTerrainCollision(bullets, permutations);

                // checkForHit(bullets, ufos, 14.0f);
                // Update Enemy Bullets
                game::checkBulletDist(enemyBullets, player);
                game::updateBullets(enemyBullets, dt);
                game::checkForBulletTerrainCollision(enemyBullets, permutations);


                UpdateContext ctx = {
                    .player = &player,
                    .bullets = &enemyBullets,
                    .worldseed = &permutations,
                    .totalTime = totalTime
                };

                // Spawn Balloons
                if (timers.getTimer("spawn_balloon")) {
                    INFO("BALLOONN");
                    spawnBalloons(player, balloons, lcg, permutations);
                }
                // Update Balloons
                for (auto &balloon: balloons)
                    balloon.update(dt, ctx);
                // Spawn Ships
                if (timers.getTimer("spawn_ship"))
                    spawnShips(player, ships, lcg, permutations);
                // Update Ships
                for (auto &ship: ships)
                    ship.update(dt, ctx);
                // Spawn Planes
                if (timers.getTimer("spawn_plane"))
                    spawnPlanes(player, planes, lcg, permutations, totalTime);
                // Update Planes
                for (auto &plane: planes)
                    plane.update(dt, ctx);
                // Spawn Barrels
                if (timers.getTimer("spawn_barrel"))
                    spawnBarrels(player, barrels, lcg, permutations);
                // Update Barrels
                for (auto &barrel: barrels)
                    barrel.update(dt, ctx);


                // destroy all destructible entity types
                game::destroyDestructibles(player, balloons, explosions, 1.0f, 48.0f, score);
                game::destroyDestructibles(player, ships,    explosions, 2.0f, 48.0f, score);
                game::destroyDestructibles(player, planes,   explosions, 1.0f, 48.0f, score);
                // Barrels are pickups, not obstacles - handled separately
                game::collectBarrels(player, barrels, score);

                // bullet hits
                game::checkForHit(bullets, balloons, 24.0f);
                game::checkForHit(bullets, ships,    32.0f);
                game::checkForHit(bullets, planes,   12.0f);
                game::checkHitForPlayer(enemyBullets, player,  12.0f);

                // rocket hits: one rocket should be decisive against anything
                // it actually reaches
                game::checkForRocketHit(rockets, planes,   explosions, 20.0f, 14);
                game::checkForRocketHit(rockets, ships,    explosions, 40.0f, 10);
                game::checkForRocketHit(rockets, balloons, explosions, 28.0f, 10);

                // AABB collision (planes only, as currently)
                game::checkForCollision(player, planes, explosions, 1.0f, glm::vec3(26.f, 26.f, 72.f));

                // pairwise self-collision
                game::checkForCollision(balloons, 24.0f);

                gui.drawHUD();

                if (free_look) {
                    window.getCamera().updateCameraFreeLook(player, static_cast<float>(Window::getMouseDX()), static_cast<float>(Window::getMouseDY()), 0.5f);
                }else {
                    window.getCamera().updateCamera(player, dt);
                }

                // to make the terrain infinite
                game::generateNewChunks(permutations, chunktables, decorations);

                gui.dItems.playerPosition = player.transform.position;
                gui.dItems.cameraPosition = window.getCamera().position;
                gui.dItems.shipCount = ships.size();
                gui.dItems.balloonCount = balloons.size();
                gui.dItems.planeCount = planes.size();
                skybox = skyboxes.at(gui.dItems.selectedSkybox);

                // Update HUD data
                gui.hudItems.health = player.health;
                gui.hudItems.speed = player.speed;
                gui.hudItems.bulletCount = bullets.size();
                gui.hudItems.altitude = player.transform.position.y;
                gui.hudItems.score = score;
                gui.hudItems.highScore = SETTINGS.getHighScore();
                gui.hudItems.newHighScore = newHighScore;
                gui.hudItems.crashed = player.crashed;
                gui.hudItems.fuel = player.fuel;
                //Time survived in this run, not time since the process started
                gui.hudItems.elapsedTime = totalTime;

            } else if (gameState == game::DEAD) {
                // Death should not be handled instantly, otherwise the player won't see the explosion
                // TRACE("Player Deathtimer: %.2f", player.deathtimer);
                if (player.deathtimer > 2.0f) {
                    switch (gui.drawDeathMenu()) {
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
                            rockets.clear();
                            balloons.clear();
                            ships.clear();
                            planes.clear();
                            explosions.clear();
                            barrels.clear();
                            // Reset the run itself, not just the entities: the
                            // score, the clock that drives the difficulty ramp
                            // and the spawn countdowns all start over
                            score = 0;
                            totalTime = 0.0f;
                            newHighScore = false;
                            targetLock.clear();
                            timers.resetAll();
                            // Put the camera back behind the new plane so the
                            // first frame does not lerp in from the crash site
                            window.getCamera().updateCamera(player);
                            gameState = game::RUNNING;
                            break;
                        default:
                            break;
                    }
                } else {
                    player.update(dt);
                    updateExplosions(explosions, player.transform.position, dt);
                    gfx::displayExplosions(explosions);
                }
            } else if (gameState == game::PAUSED) {
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
            if (gui.dItems.isShakeButtonPressed || window.getKeyState(SDLK_m) == JUST_PRESSED) {
                window.getCamera().shakeCamera(gui.dItems.shakeIntensity, gui.dItems.shakeDuration);
            }
            if (window.getKeyState(SDLK_r) == JUST_PRESSED) draw_rain = !draw_rain;
            draw_rain = gui.dItems.drawRain;

            if (window.getKeyState(SDLK_TAB) == JUST_PRESSED)
                draw_debug_gui = !draw_debug_gui;
            if (window.getKeyState(SDLK_ESCAPE) == JUST_PRESSED) {
                if (gameState == game::RUNNING) gameState = game::PAUSED;
                else if (gameState == game::PAUSED) gameState = game::RUNNING;
            }

            if (window.getKeyState(SDLK_p) == JUST_PRESSED) {
                static unsigned int i = 0;
                i = (i + 1) % skyboxes.size();
                gui.dItems.selectedSkybox = i;
                INFO("selected skybox index = %d", i);
            }

            if (window.getKeyState(SDLK_c) == JUST_PRESSED) {
                free_look = !free_look;
                if (free_look) {
                    window.getCamera().initFreeLookFromPlayer(player);
                }else {
                    window.getCamera().freeLook = false;
                }
            }

            if (draw_debug_gui) {
                gui.drawUI();
            }

            gui.render();

            window.swapBuffers();
            window.updateKeyStates();
            dt = clampDt(getTime() - startTime);
            timers.reset();
        }

        return NONE;
    }
}
