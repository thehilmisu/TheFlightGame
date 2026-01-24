#include "game.h"
#include "window.h"
// #include "audio.hpp"
#include <algorithm>

namespace gobjs = gameobjects;

namespace gameobjects {
Props::Props(glm::vec3 position, int hp, unsigned int scoreval) {
  transform.position = position;
  transform.scale = glm::vec3(1.0f);
  transform.rotation = glm::vec3(0.0f);
  hitpoints = hp;
  scorevalue = scoreval;
}

float Props::getVal(const std::string &key) const {
  if (!values.count(key))
    return 0.0f;
  return values.at(key);
}

void Props::setVal(const std::string &key, float v) { values[key] = v; }
} // namespace gameobjects

namespace game {
void destroyProps(gobjs::Player &player, std::vector<gobjs::Props> &props,
                    std::vector<gobjs::Explosion> &explosions,
                    float explosionscale, float crashdist,
                    unsigned int &score) {
  if (props.empty())
    return;

  props.erase(
      std::remove_if(props.begin(), props.end(),
                     [&player, &explosions, &crashdist, &score,
                      &explosionscale](gobjs::Props &props) {
                       if (props.hitpoints <= 0) {
                         // SNDSRC->playid("explosion",
                         // enemy.transform.position);
                         explosions.push_back(gobjs::Explosion(
                             props.transform.position, explosionscale));
                         score += props.scorevalue;
                         return true;
                       }

                       glm::vec3 diff =
                           props.transform.position - player.transform.position;

                       if (glm::length(diff) < crashdist && !player.crashed) {
                         // SNDSRC->playid(
                         // 	"explosion",
                         // 	enemy.transform.position,
                         // 	explosionscale
                         // );
                         player.crashed = true;
                         explosions.push_back(gobjs::Explosion(
                             props.transform.position, explosionscale));
                         return true;
                       }

                       return glm::length(diff) > CHUNK_SZ * 32.0f;
                     }),
      props.end());
}

void checkForCollision(gameobjects::Player &player,
                       std::vector<gobjs::Props> &props,
                       const glm::vec3 &extents) {
  if (props.empty())
    return;

  props.erase(
      std::remove_if(props.begin(), props.end(),
                     [&player, &extents](gobjs::Props &props) {
                       glm::vec3 diff =
                           player.transform.position - props.transform.position;
                       diff = props.transform.invRotate(diff);

                       if (std::abs(diff.x) < extents.x &&
                           std::abs(diff.y) < extents.y &&
                           std::abs(diff.z) < extents.z) {
                         player.crashed = true;
                         return true;
                       }

                       return false;
                     }),
      props.end());
}

void checkForCollision(std::vector<gameobjects::Props> &props,
                       float hitdist) {
  for (int i = 0; i < props.size(); i++) {
    for (int j = i + 1; j < props.size(); j++) {
      glm::vec3 p1 = props.at(i).transform.position,
                p2 = props.at(j).transform.position;
      float dist = glm::length(p1 - p2);

      if (dist < hitdist) {
        props.at(i).scorevalue = 0;
        props.at(i).hitpoints = 0;
        props.at(j).scorevalue = 0;
        props.at(j).hitpoints = 0;
      }
    }
  }
}
} // namespace game
