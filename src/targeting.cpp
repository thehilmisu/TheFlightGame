#include "game.h"
#include "audio.h"
#include <algorithm>

namespace gobjs = gameobjects;

//How the missile lock behaves.
//A target has to sit inside a cone in front of the nose and stay there for
//LOCK_TIME before the lock goes solid. Losing the cone drops it immediately,
//which keeps the player pointing at what they want to shoot.
constexpr float LOCK_RANGE = CHUNK_SZ * 14.0f;
constexpr float LOCK_CONE_DEGREES = 26.0f;
constexpr float LOCK_TIME = 1.1f;
//A locked target is allowed to drift a little wider before it is dropped, so a
//solid lock does not flicker off the instant the target edges out of the cone
constexpr float LOCK_HOLD_CONE_DEGREES = 38.0f;

namespace game {
    namespace {
        struct Candidate {
            unsigned int id = 0;
            glm::vec3 position = glm::vec3(0.0f);
            float score = -1.0f; //higher is better
        };

        //Considers one enemy list and keeps whichever entity is best centred
        template<typename T>
        void considerAll(
            const std::vector<T> *enemies,
            const glm::vec3 &origin,
            const glm::vec3 &facing,
            float minDot,
            Candidate &best
        ) {
            if (!enemies)
                return;

            for (const auto &enemy: *enemies) {
                if (enemy.hitpoints <= 0)
                    continue;

                const glm::vec3 offset = enemy.transform.position - origin;
                const float distance = glm::length(offset);
                if (distance > LOCK_RANGE || distance < 1.0f)
                    continue;

                const float dot = glm::dot(glm::normalize(offset), facing);
                if (dot < minDot)
                    continue;

                //Prefer targets near the centre of the cone, and among those
                //the closer one. Both terms are in 0..1.
                const float centring = (dot - minDot) / std::max(1.0f - minDot, 0.0001f);
                const float closeness = 1.0f - distance / LOCK_RANGE;
                const float score = centring * 0.7f + closeness * 0.3f;

                if (score > best.score) {
                    best.id = enemy.id;
                    best.position = enemy.transform.position;
                    best.score = score;
                }
            }
        }
    }

    void updateTargetLock(
        TargetLock &lock,
        const gobjs::Player &player,
        const TargetDirectory &targets,
        float dt
    ) {
        if (player.crashed) {
            lock.clear();
            return;
        }

        const glm::vec3 origin = player.transform.position;
        const glm::vec3 facing = player.transform.direction();

        //If something is already being tracked, see whether it is still there
        //and still roughly ahead of us before looking for anything new
        if (lock.tracking()) {
            glm::vec3 position;
            if (targets.findPosition(lock.targetId, position)) {
                const glm::vec3 offset = position - origin;
                const float distance = glm::length(offset);
                const float holdDot = std::cos(glm::radians(
                    lock.locked ? LOCK_HOLD_CONE_DEGREES : LOCK_CONE_DEGREES));

                if (distance <= LOCK_RANGE && distance > 1.0f &&
                    glm::dot(glm::normalize(offset), facing) >= holdDot) {
                    lock.position = position;
                    if (!lock.locked) {
                        lock.progress += dt / LOCK_TIME;
                        if (lock.progress >= 1.0f) {
                            lock.progress = 1.0f;
                            lock.locked = true;
                            SNDSRC->playid("click", origin);
                        }
                    }
                    return;
                }
            }
            //Target destroyed, out of range or out of the cone
            lock.clear();
        }

        //Acquire something new
        Candidate best;
        const float minDot = std::cos(glm::radians(LOCK_CONE_DEGREES));
        considerAll(targets.planes, origin, facing, minDot, best);
        considerAll(targets.ships, origin, facing, minDot, best);
        considerAll(targets.balloons, origin, facing, minDot, best);

        if (best.id != 0) {
            lock.targetId = best.id;
            lock.position = best.position;
            lock.progress = 0.0f;
            lock.locked = false;
        }
    }
}
