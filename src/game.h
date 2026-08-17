#pragma once
#include <iostream>
#include "infworld.h"
#include "transform.h"
#include "entity.h"
#include "logger.h"

//Constants
constexpr float SPEED = 48.0f;
constexpr float ACCELERATION = 12.0f;
constexpr float BULLET_SPEED = 384.0f;
constexpr float ROCKET_SPEED = 84.0f;
constexpr unsigned int MAX_LOD = 5;
constexpr float LOD_SCALE = 2.0f;

constexpr float FOVY = glm::radians(75.0f);
constexpr float ZNEAR = 2.0f;
constexpr float ZFAR = 20000.0f;

constexpr int RANGE = 4;

//Rockets carried at the start of a run
constexpr unsigned int MAX_ROCKETS = 3;

const glm::vec3 LIGHT = glm::normalize(glm::vec3(-1.0f));

namespace game {
    enum GameState {
        RUNNING,
        PAUSED,
        DEAD,
        GAME_EXIT_TO_MAINMENU,
        GAME_EXIT
    };

    enum MainMenuActions {
        NONE_SELECTED,
        START_GAME,
        HANGAR,
        LOAD_GAME,
        OPTIONS,
        CREDITS,
        CHANGE_PLANE_MINUS,
        CHANGE_PLANE_PLUS,
        PLANE_SELECTED,
        BACK_TO_MAINMENU,
        EXIT_GAME,
    };

    enum PauseMenuActions {
        NONE,
        RESUME,
        EXIT,
        EXIT_TO_MAINMENU,
    };

    enum DeathMenuActions {
        DEATH_NONE,
        TRY_AGAIN,
        DEATH_EXIT,
        DEATH_EXIT_TO_MAINMENU,
    };

    //Resolves an entity id to a live position. The enemy vectors are erased and
    //reordered every frame, so anything that needs to follow one particular
    //enemy across frames (a rocket, a target lock) stores its id and looks it
    //up through here rather than keeping an index or a pointer.
    struct TargetDirectory {
        const std::vector<gameobjects::Plane> *planes = nullptr;
        const std::vector<gameobjects::Ship> *ships = nullptr;
        const std::vector<gameobjects::Balloon> *balloons = nullptr;

        //False when that entity no longer exists, i.e. it has been destroyed
        [[nodiscard]] bool findPosition(unsigned int id, glm::vec3 &position) const;
    };

    //State of the player's missile lock
    struct TargetLock {
        unsigned int targetId = 0;              //0 means nothing is tracked
        glm::vec3 position = glm::vec3(0.0f);   //last known target position
        float progress = 0.0f;                  //0..1, 1 once the lock is solid
        bool locked = false;

        [[nodiscard]] bool tracking() const { return targetId != 0; }
        void clear();
    };

    //Picks the best enemy inside the firing cone and advances or drops the
    //lock. Call once per frame while the game is running.
    void updateTargetLock(
        TargetLock &lock,
        const gameobjects::Player &player,
        const TargetDirectory &targets,
        float dt
    );

    void loadAssets();

    //Initializes the shader uniforms
    void initUniforms();

    void generateChunks(const infworld::worldseed &permutations, infworld::ChunkTable *chunktables, unsigned int range);

    void generateNewChunks(const infworld::worldseed &permutations, infworld::ChunkTable *chunktables,
                           infworld::DecorationTable &decorations);

    //Game Loop. Returns the action from the pause menu
    game::PauseMenuActions arcadeModeGameLoop();

    //Development Purposes
    game::PauseMenuActions devModeGameLoop();

    //Hangar Loop
    game::PauseMenuActions hangarGameLoop();

    // Options Loop
    game::MainMenuActions optionsGameLoop();

    //Main menu
    //Returns the game mode selected
    game::MainMenuActions mainMenu();

    // Exit the Game
    void exitGame();
} // namespace game

namespace gameobjects {
    struct Player {
        game::Transform transform;
        bool crashed;

        enum {
            RY_LEFT,
            RY_RIGHT,
            RY_NONE,
        } yRotationDirection;

        enum {
            RX_UP,
            RX_DOWN,
            RX_NONE,
        } xRotationDirection;

        float deathtimer = 0.0f; //Keeps track of how long the player has been dead
        float shoottimer = 0.0f;
        float rockettimer = 0.0f;
        float speed = 0.0f;
        float currentRollRate = 0.0f;
        float currentPitchRate = 0.0f;
        float currentYawRate = 0.0f;
        float verticalVelocity = 0.0f;
        float fuel = 0.0f;
        //This is used to keep track of the time left for displaying a red screen
        //to indicate to the player that they took damage
        float damagetimer = 0.0f;
        //This keeps track of how long the player has damage immunity
        float damagecooldown = 0.0f;
        unsigned int health;
        //Rockets left in the rack. Not replenished during a run.
        unsigned int rockets = 0;

        explicit Player(glm::vec3 position);

        //Where a spinning propeller sits in the plane's own model space. The
        //placement used to be hardcoded to the Douglas's nose, which put the
        //propeller halfway down the fuselage on any other model, and gave
        //twin-engine aircraft a single propeller in the wrong place.
        struct PropellerMount {
            glm::vec3 position;
            float scale;
        };

        struct PlayerModel {
            std::string name;
            std::string plane_name;
            std::string description;
            //TODO: maybe attributes?
            float scale;
            //Empty for a model that draws its own propellers or has none
            std::vector<PropellerMount> propellers;
        };

        std::vector<PlayerModel> player_models;
        int current_model;

        void damage(unsigned int amount);

        //Adds fuel, never past a full tank
        void refuel(float amount);

        //True once the tank is empty - the engine is dead and the plane is
        //gliding down whether the player likes it or not
        [[nodiscard]] bool engineIsOut() const { return fuel <= 0.0f; }

        //Returns the percentage of health left, rounded down
        unsigned int hpPercent();

        //Returns the damagetimer amount left
        float damageTimerProgress();

        void rotateWithMouse(float dt);

        void update(float dt);

        void resetShootTimer();

        void resetRocketTimer();

        void checkIfCrashed(float dt, const infworld::worldseed &permutations);

        //Out of range indices fall back to the first plane
        void setPlayerObj(int current);

        PlayerModel getPlayerObj() { return player_models[current_model]; }
        [[nodiscard]] int getCurrentIndex() const { return current_model; }
        [[nodiscard]] int getModelCount() const { return static_cast<int>(player_models.size()); }
    };

    struct Explosion {
        game::Transform transform;
        float timePassed;
        float explosionScale;
        bool visible;

        Explosion(glm::vec3 position);

        Explosion(glm::vec3 position, float scale);

        void update(float dt);
    };

    struct Bullet {
        bool destroyed = false;
        game::Transform transform;
        float time;
        float speed;

        Bullet(const Player &player, const glm::vec3 &offset);

        Bullet(const game::Transform &t, float addspeed, const glm::vec3 &offset);

        Bullet();

        void update(float dt);
    };

    struct Rocket {
        bool destroyed = false;
        game::Transform transform;
        float time;
        float speed;
        //Entity this rocket is chasing. 0 means it was fired without a lock and
        //flies straight, which is also what happens once a chased target dies.
        unsigned int targetId = 0;

        Rocket(const Player &player, const glm::vec3 &offset, unsigned int target = 0);

        Rocket(const game::Transform &t, float addspeed, const glm::vec3 &offset);

        Rocket();

        //Steers toward the tracked target, if it still exists
        void update(float dt, const game::TargetDirectory &targets);
    };


    Balloon spawnBalloon(const glm::vec3 &position, const infworld::worldseed &permutations);

    Blimp spawnBlimp(const glm::vec3 &position, float rotation);

    Ship spawnShip(const glm::vec3 &position, const infworld::worldseed &permutations);

    Ufo spawnUfo(const glm::vec3 &position, float rotation, const infworld::worldseed &permutations);

    Plane spawnPlane(const glm::vec3 &position, float rotation, const infworld::worldseed &permutations);

    Barrel spawnBarrel(const glm::vec3 &position, const infworld::worldseed &permutations);
} // namespace gameobjects

namespace game {
    //Spawns barrels around the player
    void spawnBarrels(
        gameobjects::Player &player,
        std::vector<gameobjects::Barrel> &barrels,
        std::minstd_rand0 &lcg,
        const infworld::worldseed &permutations
    );

    //Spawns balloons around the player
    void spawnBalloons(
        gameobjects::Player &player,
        std::vector<gameobjects::Balloon> &balloons,
        std::minstd_rand0 &lcg,
        const infworld::worldseed &permutations
    );

    //Spawns ships around the player
    void spawnShips(
        gameobjects::Player &player,
        std::vector<gameobjects::Ship> &ships,
        std::minstd_rand0 &lcg,
        const infworld::worldseed &permutations
    );

    //Spawns blimps around the player
    void spawnBlimps(
        gameobjects::Player &player,
        std::vector<gameobjects::Blimp> &blimps,
        std::minstd_rand0 &lcg
    );

    //Spawn ufos around the player
    void spawnUfos(
        gameobjects::Player &player,
        std::vector<gameobjects::Ufo> &ufos,
        std::minstd_rand0 &lcg,
        const infworld::worldseed &permutations
    );

    //Spawn planes around the player
    void spawnPlanes(
        gameobjects::Player &player,
        std::vector<gameobjects::Plane> &planes,
        std::minstd_rand0 &lcg,
        const infworld::worldseed &permutations,
        float totalTime
        );

    template<typename T>
    void destroyDestructibles(gameobjects::Player& player, std::vector<T>& entities,
                                std::vector<gameobjects::Explosion>& explosions, float explosionscale,
                                float crashdist, unsigned int& score)
    {
        static_assert(std::is_base_of_v<gameobjects::DamageableEntity, T>,
                      "T must derive from DamageableEntity");
        if (entities.empty()) return;

        entities.erase(
            std::remove_if(entities.begin(), entities.end(),
                [&](T& e) {
                    if (e.hitpoints <= 0) {
                        explosions.emplace_back(e.transform.position, explosionscale);
                        score += e.scorevalue;
                        return true;
                    }
                    glm::vec3 diff = e.transform.position - player.transform.position;
                    if (glm::length(diff) < crashdist && !player.crashed) {
                        player.crashed = true;
                        explosions.emplace_back(e.transform.position, explosionscale);
                        return true;
                    }
                    return glm::length(diff) > CHUNK_SZ * 32.0f;
                }),
            entities.end());
    }


    // AABB collision between player and entities — crashes player on overlap.
    // Replaces checkForCollision(Player, vector<Enemy>, ...) and
    //                            checkForCollision(Player, vector<Props>, ...).
    template<typename T>
    void checkForCollision(
        gameobjects::Player& player,
        std::vector<T>& entities,
        std::vector<gameobjects::Explosion>& explosions,
        float explosionscale,
        const glm::vec3& extents)
    {
        static_assert(std::is_base_of_v<gameobjects::DamageableEntity, T>,
                      "T must derive from DamageableEntity");
        if (entities.empty()) return;

        entities.erase(
            std::remove_if(entities.begin(), entities.end(),
                [&](T& e) {
                    glm::vec3 diff = player.transform.position - e.transform.position;
                    diff = e.transform.invRotate(diff);
                    if (std::abs(diff.x) < extents.x &&
                        std::abs(diff.y) < extents.y &&
                        std::abs(diff.z) < extents.z) {
                        player.crashed = true;
                        explosions.emplace_back(e.transform.position, explosionscale);
                        return true;
                    }
                    return false;
                }),
            entities.end());
    }

    // Pairwise sphere collision among entities of the same type —
    // sets both to dead (hitpoints=0, scorevalue=0) on contact.
    // Replaces checkForCollision(vector<Enemy>, float) and
    //                            checkForCollision(vector<Props>, float).
    template<typename T>
    void checkForCollision(std::vector<T>& entities, float hitdist)
    {
        static_assert(std::is_base_of_v<gameobjects::DamageableEntity, T>,
                      "T must derive from DamageableEntity");
        for (size_t i = 0; i < entities.size(); i++) {
            for (size_t j = i + 1; j < entities.size(); j++) {
                const float dist = glm::length(
                    entities[i].transform.position - entities[j].transform.position);
                if (dist < hitdist) {
                    entities[i].hitpoints  = 0;  entities[i].scorevalue = 0;
                    entities[j].hitpoints  = 0;  entities[j].scorevalue = 0;
                }
            }
        }
    }

    // Bullet-vs-entity hit detection. Marks bullet destroyed and
    // decrements entity hitpoints on hit.
    // Replaces checkForHit(vector<Bullet>, vector<Enemy>, float).
    template<typename T>
    void checkForHit(
        std::vector<gameobjects::Bullet>& bullets,
        std::vector<T>& enemies,
        float hitdist)
    {
        static_assert(std::is_base_of_v<gameobjects::DamageableEntity, T>,
                      "T must derive from DamageableEntity");
        for (auto& bullet : bullets) {
            for (auto& enemy : enemies) {
                const float dist = glm::length(
                    bullet.transform.position - enemy.transform.position);
                if (dist < hitdist) {
                    bullet.destroyed = true;
                    enemy.hitpoints--;
                    TRACE("You have hit the enemy with bullets!");
                }
            }
        }
    }

    inline void checkHitForPlayer(
        std::vector<gameobjects::Bullet>& bullets,
        gameobjects::Player& player,
        float hitdist)
    {
        for (auto& bullet : bullets) {
            float dist = glm::length(
                   bullet.transform.position - player.transform.position);
            if (dist < hitdist) {
                bullet.destroyed = true;
                player.damage(5);
                TRACE("You have been hit! Healtgh: %zu", player.health);
            }
        }
    }

    //Barrels are fuel pickups rather than obstacles: flying into one tops the
    //tank up instead of destroying the player, so they deliberately do not go
    //through destroyDestructibles like the other entity types
    void collectBarrels(
        gameobjects::Player &player,
        std::vector<gameobjects::Barrel> &barrels,
        unsigned int &score
    );

    //Update explosions
    void updateExplosions(
        std::vector<gameobjects::Explosion> &explosions,
        const glm::vec3 &center,
        float dt
    );

    //Check if the bullet is too far away from the player, if it is, then
    //mark it to be deleted
    //This should preferably be called before `updateBullets`
    void checkBulletDist(
        std::vector<gameobjects::Bullet> &bullets,
        const gameobjects::Player &player
    );

    void updateBullets(std::vector<gameobjects::Bullet> &bullets, float dt);

    void checkForBulletTerrainCollision(
        std::vector<gameobjects::Bullet> &bullets,
        infworld::worldseed &permutations
    );

    void updateRockets(
        std::vector<gameobjects::Rocket> &rockets,
        float dt,
        const TargetDirectory &targets
    );

    //Rockets that fly into the ground go off there
    void checkForRocketTerrainCollision(
        std::vector<gameobjects::Rocket> &rockets,
        std::vector<gameobjects::Explosion> &explosions,
        infworld::worldseed &permutations
    );

    // Rocket-vs-entity hit detection. A rocket is a single heavy hit rather
    // than the chip damage a bullet does, and it always detonates.
    template<typename T>
    void checkForRocketHit(
        std::vector<gameobjects::Rocket>& rockets,
        std::vector<T>& enemies,
        std::vector<gameobjects::Explosion>& explosions,
        float hitdist,
        int damage)
    {
        static_assert(std::is_base_of_v<gameobjects::DamageableEntity, T>,
                      "T must derive from DamageableEntity");
        if (rockets.empty() || enemies.empty()) return;

        for (auto& rocket : rockets) {
            if (rocket.destroyed) continue;
            for (auto& enemy : enemies) {
                if (enemy.hitpoints <= 0) continue;
                const float dist = glm::length(
                    rocket.transform.position - enemy.transform.position);
                if (dist < hitdist) {
                    rocket.destroyed = true;
                    enemy.hitpoints -= damage;
                    explosions.emplace_back(rocket.transform.position, 2.0f);
                    TRACE("Rocket hit its target!");
                    break;
                }
            }
        }
    }
} // namespace game

namespace gfx {
    void displaySkybox(const std::string &skybox);

    void displayWater(float totalTime);

    void displayDecorations(infworld::DecorationTable &decorations, float totalTime);

    unsigned int displayTerrain(infworld::ChunkTable *chunktables, int maxlod, float lodscale);

    void generateDecorationOffsets(infworld::DecorationTable &decorations);

    void displayPlayerPlane(float totalTime, const game::Transform &transform,
                            const gameobjects::Player::PlayerModel &plane_model);

    void displayExplosions(const std::vector<gameobjects::Explosion> &explosions);

    void displayBalloons(const std::vector<gameobjects::Balloon> &balloons);

    void displayBarrels(const std::vector<gameobjects::Barrel> &barrels);

    void displayShips(const std::vector<gameobjects::Ship> &ships);

    void displayBlimps(const std::vector<gameobjects::Blimp> &blimps);

    void displayUfos(const std::vector<gameobjects::Ufo> &ufos);

    void displayPlanes(float totalTime, const std::vector<gameobjects::Plane> &planes);

    void displayBullets(const std::vector<gameobjects::Bullet> &bullets);

    void displayRockets(const std::vector<gameobjects::Rocket> &rockets);

    void displayMiniMapBackground(float totalTime);

    void displayAttitude(float pitch, float roll);

    void displaySpeed(float speed);

    void displayDanger(float totalTime);

    void displayRain(float totalTime);

    void displayPlaneHealth(float health, float totalTime);

    void displayFuel(float fuel, float totalTime);

    void displayDashboardBackground();

    //Draws a dot on the minimap for each enemy within range.
    //This takes the concrete entity type rather than DamageableEntity: a
    //vector<Balloon> is not a vector<DamageableEntity>, which is why the call
    //sites were commented out after the entity refactor. Instantiated for
    //Balloon, Ship and Plane in display.cpp.
    template<typename T>
    void displayEnemyMarkers(
        const std::vector<T> &enemies,
        const game::Transform &playertransform,
        const glm::vec3 &color
    );

    void displayCrosshair(const game::Transform &playertransform);

    //Corner brackets around the tracked target that tighten as the lock builds
    void displayTargetLock(const game::TargetLock &lock);

    void displayHUDBackGrounds();

    //Rack of rocket icons: filled for loaded, dark for spent
    void displayRocketAmmo(unsigned int remaining, unsigned int capacity);

    void displayHangar(float angle = 0.0f);

    void displayFog();
} // namespace gfx
