#include "game.h"

namespace gobjs = gameobjects;

namespace gameobjects {
void Props::updateBarrel(float dt) {
  if (transform.position.y > values.at("maxy")) {
    transform.position.y = values.at("maxy");
    values.at("direction") *= -1.0f;
  } else if (transform.position.y < values.at("miny")) {
    transform.position.y = values.at("miny");
    values.at("direction") *= -1.0f;
  }

  transform.position.y += 16.0f * dt * values.at("direction");
}

Props spawnBarrel(const glm::vec3 &position,
                   const infworld::worldseed &permutations) {
  float h =
      infworld::getHeight(position.z / SCALE * float(PREC + 1) / float(PREC),
                          position.x / SCALE * float(PREC + 1) / float(PREC),
                          permutations) *
      HEIGHT * SCALE;
  float y = std::max(h, 0.0f) + HEIGHT;
  glm::vec3 pos(position.x, y, position.z);
  Props barrel = Props(pos, 5, 10);

  float miny = y;
  float maxy = y + HEIGHT;
  float direction = 1.0f;
  barrel.setVal("miny", miny);
  barrel.setVal("maxy", maxy);
  barrel.setVal("direction", direction);
  return barrel;
}
} // namespace gameobjects

namespace game {
void spawnBarrels(gobjs::Player &player, std::vector<gobjs::Props> &barrels,
                   std::minstd_rand0 &lcg,
                   const infworld::worldseed &permutations) {
  if (barrels.size() >= 4)
    return;

  int randval = lcg() % 2;
  if (randval > 0 && barrels.size() > 1)
    return;

  glm::vec3 center = player.transform.position;
  float dist = float(lcg() % 256) / 256.0f * CHUNK_SZ * 12.0f + CHUNK_SZ * 6.0f;
  float angle = float(lcg() % 256) / 256.0f * glm::radians(360.0f);
  glm::vec3 position =
      center + dist * glm::vec3(cosf(angle), 0.0f, sinf(angle));
  barrels.push_back(gobjs::spawnBarrel(position, permutations));
}
} // namespace game
