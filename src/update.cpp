#include "game.h"
#include "logger.h"
#include "window.h"
#include "audio.h"
#include "flightmath.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace gobjs = gameobjects;

//How close the player has to get to a barrel to pick it up, and how much fuel
//it gives back. A full tank is 100, so a barrel is roughly a quarter tank.
constexpr float BARREL_PICKUP_DIST = 32.0f;
constexpr float BARREL_FUEL = 25.0f;

//Rocket flight profile. The rocket coasts briefly after launch, then the motor
//lights and it accelerates hard while steering toward whatever it locked onto.
constexpr float ROCKET_IGNITION_DELAY = 0.22f;  //seconds of unpowered coast
constexpr float ROCKET_DROP_RATE = 1.1f;        //rad/s the nose falls while coasting
constexpr float ROCKET_COAST_DRAG = 26.0f;
constexpr float ROCKET_ACCELERATION = 190.0f;
constexpr float ROCKET_MAX_SPEED = 340.0f;
constexpr float ROCKET_TURN_RATE = 2.4f;        //rad/s, the limit on how hard it corners
constexpr float ROCKET_LIFETIME = 8.0f;         //self destructs after this

namespace gameobjects {
    Explosion::Explosion(glm::vec3 position) {
        transform.position = position;
        transform.scale = glm::vec3(1.0f);
        transform.rotation = glm::vec3(0.0f);
        timePassed = 0.0f;
        explosionScale = 1.0f;
        visible = true;
    }

    Explosion::Explosion(glm::vec3 position, float scale) {
        transform.position = position;
        transform.scale = glm::vec3(1.0f);
        explosionScale = scale;
        transform.rotation = glm::vec3(0.0f);
        timePassed = 0.0f;
        visible = true;
    }

    void Explosion::update(float dt) {
        if (!visible)
            return;

        timePassed += dt;
        transform.scale = (glm::vec3(10.0f) * timePassed + glm::vec3(1.0f)) * explosionScale;

        if (timePassed > 2.0f)
            visible = false;
    }

    Bullet::Bullet(const Player &player, const glm::vec3 &offset) {
        time = 0.0f;
        transform.position =
                player.transform.rotate(offset) +
                player.transform.position;
        transform.rotation = player.transform.rotation;
        transform.scale = glm::vec3(1.0f);
        speed = BULLET_SPEED + player.speed - SPEED;
    }

    Bullet::Bullet(const game::Transform &t, float addspeed, const glm::vec3 &offset) {
        time = 0.0f;
        transform.position =
                t.rotate(offset) +
                t.position;
        transform.rotation = t.rotation;
        transform.scale = glm::vec3(1.0f);
        speed = BULLET_SPEED + addspeed - SPEED;
    }

    Bullet::Bullet() {
        time = 0.0f;
        speed = BULLET_SPEED;
    }

    void Bullet::update(float dt) {
        time += dt;
        transform.position += transform.direction() * dt * speed;
    }

    Rocket::Rocket(const Player &player, const glm::vec3 &offset, unsigned int target) {
        time = 0.0f;
        transform.position =
                player.transform.rotate(offset) +
                player.transform.position;
        transform.rotation = player.transform.rotation;
        transform.scale = glm::vec3(1.0f);
        //Leaves the rail at the aircraft's speed and builds up from there
        speed = player.speed;
        targetId = target;
    }

    Rocket::Rocket(const game::Transform &t, float addspeed, const glm::vec3 &offset) {
        time = 0.0f;
        transform.position =
                t.rotate(offset) +
                t.position;
        transform.rotation = t.rotation;
        transform.scale = glm::vec3(1.0f);
        speed = ROCKET_SPEED + addspeed - SPEED;
    }

    Rocket::Rocket() {
        time = 0.0f;
        speed = BULLET_SPEED;
    }

    void Rocket::update(float dt, const game::TargetDirectory &targets) {
        time += dt;

        if (time < ROCKET_IGNITION_DELAY) {
            // Off the rail but not yet under power: it coasts and the nose
            // falls away from the aircraft, which is what sells the launch
            transform.rotation.x += ROCKET_DROP_RATE * dt;
            speed = std::max(speed - ROCKET_COAST_DRAG * dt, SPEED * 0.5f);
        } else {
            // Motor lit
            speed = std::min(speed + ROCKET_ACCELERATION * dt, ROCKET_MAX_SPEED);

            glm::vec3 targetpos;
            if (targetId != 0 && targets.findPosition(targetId, targetpos)) {
                const glm::vec3 offset = targetpos - transform.position;

                if (glm::length(offset) > 0.001f) {
                    // A rocket cannot turn instantly: capping the correction
                    // per frame is what lets a hard-manoeuvring target still
                    // shake it off
                    const glm::vec3 heading = flight::steerToward(
                        transform.direction(), glm::normalize(offset), ROCKET_TURN_RATE * dt);

                    const glm::vec2 euler = flight::headingToEuler(heading);
                    transform.rotation.x = euler.x;
                    transform.rotation.y = euler.y;
                }
            }
        }

        transform.position += transform.direction() * dt * speed;
    }
}

namespace game {
    void updateExplosions(
        std::vector<gobjs::Explosion> &explosions,
        const glm::vec3 &center,
        float dt
    ) {
        for (auto &explosion: explosions)
            explosion.update(dt);

        explosions.erase(std::remove_if(
                             explosions.begin(),
                             explosions.end(),
                             [](const gobjs::Explosion &explosion) {
                                 return !explosion.visible;
                             }
                         ), explosions.end());

        std::sort(
            explosions.begin(),
            explosions.end(),
            [&center](const gobjs::Explosion &e1, const gobjs::Explosion &e2) {
                const glm::vec3 d1 = e1.transform.position - center;
                const glm::vec3 d2 = e2.transform.position - center;
                return glm::length(d1) > glm::length(d2);
            }
        );
    }

    bool TargetDirectory::findPosition(unsigned int id, glm::vec3 &position) const {
        if (id == 0)
            return false;

        //Small linear scans: these lists are capped at a handful of entities
        if (planes)
            for (const auto &e: *planes)
                if (e.id == id) { position = e.transform.position; return true; }
        if (ships)
            for (const auto &e: *ships)
                if (e.id == id) { position = e.transform.position; return true; }
        if (balloons)
            for (const auto &e: *balloons)
                if (e.id == id) { position = e.transform.position; return true; }

        return false;
    }

    void TargetLock::clear() {
        targetId = 0;
        progress = 0.0f;
        locked = false;
    }

    void updateRockets(
        std::vector<gobjs::Rocket> &rockets,
        float dt,
        const TargetDirectory &targets
    ) {
        for (auto &rocket: rockets)
            rocket.update(dt, targets);

        rockets.erase(std::remove_if(
                          rockets.begin(),
                          rockets.end(),
                          [](const gobjs::Rocket &rocket) {
                              return rocket.destroyed ||
                                     rocket.transform.position.y < 0.0f ||
                                     rocket.time > ROCKET_LIFETIME;
                          }
                      ), rockets.end());
    }

    void checkForRocketTerrainCollision(
        std::vector<gobjs::Rocket> &rockets,
        std::vector<gobjs::Explosion> &explosions,
        infworld::worldseed &permutations
    ) {
        for (auto &rocket: rockets) {
            if (rocket.destroyed)
                continue;
            const glm::vec3 pos = rocket.transform.position;
            const float h = infworld::getHeight(
                                pos.z / SCALE * float(PREC + 1) / float(PREC),
                                pos.x / SCALE * float(PREC + 1) / float(PREC),
                                permutations
                            ) * HEIGHT * SCALE;
            if (pos.y < h) {
                rocket.destroyed = true;
                explosions.emplace_back(pos, 1.5f);
            }
        }
    }

    void updateBullets(std::vector<gobjs::Bullet> &bullets, float dt) {
        for (auto &bullet: bullets)
            bullet.update(dt);

        bullets.erase(std::remove_if(
                          bullets.begin(),
                          bullets.end(),
                          [](gobjs::Bullet &bullet) {
                              return bullet.destroyed || bullet.transform.position.y < 0.0f;
                          }
                      ), bullets.end());
    }

    void checkBulletDist(
        std::vector<gobjs::Bullet> &bullets,
        const gobjs::Player &player
    ) {
        for (auto &bullet: bullets) {
            glm::vec3 diff = bullet.transform.position - player.transform.position;
            float dist = glm::length(diff);
            if (dist > BULLET_SPEED * 3.0f)
                bullet.destroyed = true;
        }
    }

    void collectBarrels(
        gobjs::Player &player,
        std::vector<gobjs::Barrel> &barrels,
        unsigned int &score
    ) {
        if (barrels.empty())
            return;

        barrels.erase(std::remove_if(
                          barrels.begin(),
                          barrels.end(),
                          [&](const gobjs::Barrel &barrel) {
                              const glm::vec3 diff =
                                      barrel.transform.position - player.transform.position;
                              const float dist = glm::length(diff);

                              if (dist < BARREL_PICKUP_DIST && !player.crashed) {
                                  player.refuel(BARREL_FUEL);
                                  score += barrel.scorevalue;
                                  SNDSRC->playid("hit", barrel.transform.position);
                                  return true;
                              }

                              //Drop the ones the player has long since flown past
                              return dist > CHUNK_SZ * 32.0f;
                          }
                      ), barrels.end());
    }

    void checkForBulletTerrainCollision(
        std::vector<gobjs::Bullet> &bullets,
        infworld::worldseed &permutations
    ) {
        bullets.erase(std::remove_if(
                          bullets.begin(),
                          bullets.end(),
                          [&permutations](gobjs::Bullet &bullet) {
                              glm::vec3 pos = bullet.transform.position;
                              float h = infworld::getHeight(
                                            pos.z / SCALE * float(PREC + 1) / float(PREC),
                                            pos.x / SCALE * float(PREC + 1) / float(PREC),
                                            permutations
                                        ) * HEIGHT * SCALE;
                              return pos.y < h;
                          }
                      ), bullets.end());
    }
}
