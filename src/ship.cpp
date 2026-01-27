#include "game.h"

namespace gobjs = gameobjects;

namespace gameobjects {
void Enemy::updateShip(float dt, const Player &player, std::vector<Bullet> &bullets) {
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

  if (shootTimer > 3.0f) { // Shoot every 3 seconds
    setVal("shoottimer", 0.0f);

    // Aim at player
    glm::vec3 toPlayer = player.transform.position - transform.position;
    toPlayer = glm::normalize(toPlayer);

    // Set rotation to face player
    transform.rotation.y = atan2f(toPlayer.x, toPlayer.z);

    // Spawn rocket
    bullets.push_back(Bullet(transform, 0.0f, glm::vec3(0.0f, 5.0f, 0.0f)));
  }
}

Enemy spawnShip(const glm::vec3 &position,
                const infworld::worldseed &permutations) {
  float h =
      infworld::getHeight(position.z / SCALE * float(PREC + 1) / float(PREC),
                          position.x / SCALE * float(PREC + 1) / float(PREC),
                          permutations) *
      HEIGHT * SCALE;

  // Only spawn on water (where terrain is below water level)
  if (h >= 0.0f) {
    // Not water, return at water level anyway
    h = -10.0f;
  }

  float y = 10.0f; // Slightly above water level so ship is visible
  glm::vec3 pos(position.x, y, position.z);
  Enemy ship = Enemy(pos, 10, 50);

  float miny = y - 8.0f;
  float maxy = y + 8.0f;
  float direction = 1.0f;
  ship.setVal("miny", miny);
  ship.setVal("maxy", maxy);
  ship.setVal("direction", direction);
  ship.setVal("shoottimer", 0.0f);
  return ship;
}
} // namespace gameobjects

namespace game {
void spawnShips(gobjs::Player &player, std::vector<gobjs::Enemy> &ships,
                std::minstd_rand0 &lcg,
                const infworld::worldseed &permutations) {
  if (ships.size() >= 3)
    return;

  int randval = lcg() % 3;
  if (randval > 0 && ships.size() > 0)
    return;

  glm::vec3 center = player.transform.position;
  float dist = float(lcg() % 256) / 256.0f * CHUNK_SZ * 12.0f + CHUNK_SZ * 6.0f;
  float angle = float(lcg() % 256) / 256.0f * glm::radians(360.0f);
  glm::vec3 position =
      center + dist * glm::vec3(cosf(angle), 0.0f, sinf(angle));

  // Check if position is actually over water
  float h =
      infworld::getHeight(position.z / SCALE * float(PREC + 1) / float(PREC),
                          position.x / SCALE * float(PREC + 1) / float(PREC),
                          permutations) *
      HEIGHT * SCALE;

  // Only spawn if terrain is below water level
  if (h < 0.0f) {
    ships.push_back(gobjs::spawnShip(position, permutations));
  }
}
} // namespace game
