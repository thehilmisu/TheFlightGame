#include "game.h"

namespace gobjs = gameobjects;

namespace gameobjects {
    void Ship::update(float dt, const UpdateContext &ctx) {
        const Player &player = *ctx.player;

        // Ships bob up and down on the water
        if (transform.position.y > values.at("maxy")) {
            transform.position.y = values.at("maxy");
            values.at("direction") *= -1.0f;
        } else if (transform.position.y < values.at("miny")) {
            transform.position.y = values.at("miny");
            values.at("direction") *= -1.0f;
        }

        transform.position.y += 8.0f * dt * values.at("direction");

        // Shoot rockets at the player
        float shootTimer = getVal("shoottimer");
        shootTimer += dt;
        setVal("shoottimer", shootTimer);

        if (shootTimer > 3.0f) {
            // Shoot every 3 seconds
            setVal("shoottimer", 0.0f);

            // Aim at player
            glm::vec3 toPlayer = player.transform.position - transform.position;
            toPlayer = glm::normalize(toPlayer);

            // Set rotation to face player (yaw + pitch)
            transform.rotation.y = atan2f(toPlayer.x, toPlayer.z);
            float horizontalDist = sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
            transform.rotation.x = -atan2f(toPlayer.y, horizontalDist);

            // Spawn rocket
            ctx.bullets->emplace_back(transform, 0.3f, glm::vec3(0.0f, 5.0f, 0.0f));
        }
    }

    Ship spawnShip(const glm::vec3 &position,
                    const infworld::worldseed &permutations) {
        float h =
                infworld::getHeight(position.z / SCALE * static_cast<float>(PREC + 1) / static_cast<float>(PREC),
                                    position.x / SCALE * static_cast<float>(PREC + 1) / static_cast<float>(PREC),
                                    permutations) *
                HEIGHT * SCALE;

        // Only spawn on water (where terrain is below water level)
        if (h >= 0.0f) {
            // Not water, return at water level anyway
            h = -10.0f;
        }

        float y = 10.0f; // Slightly above water level so ship is visible
        glm::vec3 pos(position.x, y, position.z);
        auto ship = Ship(pos);

        const float miny = y - 8.0f;
        const float maxy = y + 8.0f;
        constexpr float direction = 1.0f;
        ship.setVal("miny", miny);
        ship.setVal("maxy", maxy);
        ship.setVal("direction", direction);
        ship.setVal("shoottimer", 0.0f);
        return ship;
    }
} // namespace gameobjects

namespace game {
    void spawnShips(gobjs::Player &player, std::vector<gobjs::Ship> &ships,
                    std::minstd_rand0 &lcg,
                    const infworld::worldseed &permutations) {
        if (ships.size() >= 3)
            return;

        //Randomly skip some spawn opportunities once ships are already about,
        //matching how balloons and planes are throttled. This used to test
        //ships.empty(), which threw away two thirds of the chances to spawn the
        //*first* ship and then let every later one through unconditionally.
        unsigned int randval = lcg() % 3;
        if (randval > 0 && !ships.empty())
            return;

        //Ships can only sit on water, and only about a third of the world is
        //water. Sampling a single point per attempt meant a ship appeared
        //roughly once every seven minutes of flight, so most runs never saw
        //one. Try several points and take the first that is actually at sea.
        constexpr int MAX_ATTEMPTS = 8;
        const glm::vec3 center = player.transform.position;

        for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
            const float dist =
                    static_cast<float>(lcg() % 256) / 256.0f * CHUNK_SZ * 12.0f + CHUNK_SZ * 6.0f;
            const float angle = static_cast<float>(lcg() % 256) / 256.0f * glm::radians(360.0f);
            glm::vec3 position =
                    center + dist * glm::vec3(cosf(angle), 0.0f, sinf(angle));

            // Check if position is actually over water
            const float h = infworld::getHeight(
                                position.z / SCALE * static_cast<float>(PREC + 1) / static_cast<float>(PREC),
                                position.x / SCALE * static_cast<float>(PREC + 1) / static_cast<float>(PREC),
                                permutations) * HEIGHT * SCALE;

            // Only spawn if terrain is below water level
            if (h < 0.0f) {
                auto ship = gobjs::spawnShip(position, permutations);
                //Pick which hull model this one draws with (see displayShips)
                ship.setVal("variant", static_cast<float>(lcg() % 2));
                ships.push_back(ship);
                return;
            }
        }
    }
} // namespace game
