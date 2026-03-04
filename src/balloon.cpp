#include "game.h"

namespace gobjs = gameobjects;

namespace gameobjects {
    void Balloon::update(float dt, const UpdateContext& ctx) {
        (void)ctx;

        if (transform.position.y > values.at("maxy")) {
            transform.position.y = values.at("maxy");
            values.at("direction") *= -1.0f;
        } else if (transform.position.y < values.at("miny")) {
            transform.position.y = values.at("miny");
            values.at("direction") *= -1.0f;
        }

        transform.position.y += 16.0f * dt * values.at("direction");
    }

    Balloon spawnBalloon(const glm::vec3 &position,
                       const infworld::worldseed &permutations) {
        const float h =
                infworld::getHeight(position.z / SCALE * static_cast<float>(PREC + 1) / static_cast<float>(PREC),
                                    position.x / SCALE * static_cast<float>(PREC + 1) / static_cast<float>(PREC),
                                    permutations) *
                HEIGHT * SCALE;
        float y = std::max(h, 0.0f) + HEIGHT;
        const glm::vec3 pos(position.x, y, position.z);
        auto balloon = Balloon(pos);

        const float miny = y;
        const float maxy = y + HEIGHT;
        constexpr float direction = 1.0f;
        balloon.setVal("miny", miny);
        balloon.setVal("maxy", maxy);
        balloon.setVal("direction", direction);
        return balloon;
    }
} // namespace gameobjects

namespace game {
    void spawnBalloons(gobjs::Player &player, std::vector<gobjs::Balloon> &balloons,
                       std::minstd_rand0 &lcg,
                       const infworld::worldseed &permutations) {
        if (balloons.size() >= 4)
            return;

        if (const int randval = lcg() % 2; randval > 0 && balloons.size() > 1)
            return;

        glm::vec3 center = player.transform.position;
        const float dist = static_cast<float>(lcg() % 256) / 256.0f * CHUNK_SZ * 12.0f + CHUNK_SZ * 6.0f;
        const float angle = static_cast<float>(lcg() % 256) / 256.0f * glm::radians(360.0f);
        const glm::vec3 position =
                center + dist * glm::vec3(cosf(angle), 0.0f, sinf(angle));
        balloons.push_back(gobjs::spawnBalloon(position, permutations));
    }
} // namespace game
