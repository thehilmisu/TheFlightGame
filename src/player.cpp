#include "game.h"
#include "window.h"
#include <SDL.h>
// #include "audio.hpp"

constexpr unsigned int DEFAULT_HEALTH = 100;
constexpr float DEFAULT_FUEL = 100.0f;
constexpr float DAMAGE_COOLDOWN = 0.2f;
constexpr float DAMAGE_TIMER = 1.0f;
// Roll (banking) parameters
constexpr float ROLL_RATE = 2.5f;                          // rad/s input roll speed
constexpr float MAX_ROLL_ANGLE = glm::radians(75.0f);      // max bank angle
constexpr float ROLL_RETURN_RATE = 1.2f;                    // rad/s auto-level roll
// Pitch parameters
constexpr float PITCH_RATE = 1.2f;                          // rad/s input pitch speed
constexpr float MAX_PITCH_ANGLE = glm::radians(70.0f);      // max pitch angle
constexpr float PITCH_RETURN_RATE = 0.3f;                   // rad/s gentle auto-level pitch
// Yaw derived from banking
constexpr float BANK_TURN_FACTOR = 1.0f;                    // yaw = sin(roll) * this * speedFactor
// Physics
constexpr float GRAVITY = 18.0f;                            // units/s^2 arcade gravity
constexpr float LIFT_FACTOR = 1.0f;                         // lift = gravity at cruise, wings level
constexpr float PITCH_SPEED_TRANSFER = 15.0f;               // speed gain/loss from pitch
// Smoothing (lerp factors per second)
constexpr float ROLL_SMOOTHING = 6.0f;
constexpr float PITCH_SMOOTHING = 5.0f;
constexpr float YAW_SMOOTHING = 6.0f;
// Safety
constexpr float MIN_HEIGHT = 4.0f;

constexpr float FUEL_CONSUMPTION_RATE = 2.0f;

namespace gameobjects {
Player::Player(glm::vec3 position) {
  transform.position = position;
  transform.rotation = glm::vec3(0.0f);
  transform.scale = glm::vec3(2.7f);
  xRotationDirection = RX_NONE;
  yRotationDirection = RY_NONE;
  crashed = false;
  speed = SPEED;
  health = DEFAULT_HEALTH;
  fuel = DEFAULT_FUEL;

  PlayerModel model_douglas = {
    .name = "plane_douglas",
    .plane_name = "Douglas A-01",
    .description = "Douglas A-01, default player plane.",
    .scale = 2.7f
  };
  PlayerModel model_default = {
    .name = "plane",
    .plane_name = "Temporary plane model",
    .description = "Temporary",
    .scale = 1.0f
  };
  PlayerModel model_sm79t = {
    .name = "plane_savoia",
    .plane_name = "Savoia Marchetti-sm-79-t",
    .description = "Savoia-marchetti-sm-79-t fast plane",
    .scale = 2.0f
  };
  PlayerModel model_grumman = {
    .name = "plane_grumman",
    .plane_name = "Grumman-f4f-wildcat",
    .description = "Grumman-f4f-wildcat warrior plane",
    .scale = 2.5f
  };
  PlayerModel model_phantom= {
    .name = "plane_phantom",
    .plane_name = "F4-Phantom",
    .description = "F4 Phantom warrior plane",
    .scale = 2.5f
  };
  player_models.push_back(model_douglas);
  player_models.push_back(model_default);
  player_models.push_back(model_sm79t);
  player_models.push_back(model_grumman);
  player_models.push_back(model_phantom);
  current_model = 0;
}

void Player::setPlayerObj(int current) {
  current_model = current;
  transform.scale = glm::vec3(player_models.at(current_model).scale);
}

void Player::damage(unsigned int amount) {
  if (damagecooldown > 0.0f)
    return;

  if (health < amount)
    health = 0;
  else
    health -= amount;

  // if(health > 0)
  //  SNDSRC->playid("hit", transform.position);

  damagecooldown = DAMAGE_COOLDOWN;
  damagetimer = DAMAGE_TIMER;
  std::cout << "Player took " << amount << " damage, health now " << health
            << "\n";
}

unsigned int Player::hpPercent() {
  return (unsigned int)(float(health) / float(DEFAULT_HEALTH) * 100.0f);
}

float Player::damageTimerProgress() { return damagetimer / DAMAGE_TIMER; }

void Player::rotateWithMouse(float dt) {
  Window &window = Window::getInstance();
  double dx = window.getMouseDX(), dy = window.getMouseDY();

  float speedx = 0.0f, speedy = BANK_TURN_FACTOR / 5.0f * dx;

  speedx = -PITCH_RATE * dy;

  speedx = std::min(speedx, PITCH_RATE / 2.0f);
  speedx = std::max(speedx, -PITCH_RATE / 2.0f);
  speedy = std::min(speedy, BANK_TURN_FACTOR / 3.0f);
  speedy = std::max(speedy, -BANK_TURN_FACTOR / 3.0f);

  if (yRotationDirection == Player::RY_NONE)
    transform.rotation.y -= speedy * dt;

  if (xRotationDirection == Player::RX_NONE) {
    transform.rotation.x -= speedx * dt;
    transform.rotation.x = std::min(transform.rotation.x, MAX_PITCH_ANGLE);
    transform.rotation.x = std::max(transform.rotation.x, -MAX_PITCH_ANGLE);
  }
}

void Player::update(float dt) {
  // Keep track of how long the player has crashed,
  // this is so that the game can delay showing the death screen until
  // the explosion animation is done playing
  if (crashed) {
    deathtimer += dt;
    return;
  }

  // Shooting cooldown
  shoottimer -= dt;
  // Damage cooldown
  damagecooldown -= dt;
  damagetimer -= dt;

  // Fuel Consumption
  fuel -= dt * FUEL_CONSUMPTION_RATE / 2.0f; 
  if (fuel < 0.0f)
    fuel = 0.0f;

  Window &window = Window::getInstance();

  // --- Input: same keys as before ---
  // A/D now control roll (banking), not direct yaw
  if (window.getKeyState(SDLK_d) == JUST_PRESSED)
    yRotationDirection = Player::RY_RIGHT;
  else if (window.getKeyState(SDLK_a) == JUST_PRESSED)
    yRotationDirection = Player::RY_LEFT;
  else if (window.getKeyState(SDLK_a) == RELEASED &&
           window.getKeyState(SDLK_d) == RELEASED)
    yRotationDirection = Player::RY_NONE;

  // W/S control pitch
  if (window.getKeyState(SDLK_s) == JUST_PRESSED)
    xRotationDirection = Player::RX_UP;
  else if (window.getKeyState(SDLK_w) == JUST_PRESSED)
    xRotationDirection = Player::RX_DOWN;
  else if (window.getKeyState(SDLK_s) == RELEASED &&
           window.getKeyState(SDLK_w) == RELEASED)
    xRotationDirection = Player::RX_NONE;

  // --- Compute target rotation rates ---
  float targetRollRate = 0.0f;
  float targetPitchRate = 0.0f;

  // Roll input
  if (yRotationDirection == Player::RY_RIGHT)
    targetRollRate = ROLL_RATE;
  else if (yRotationDirection == Player::RY_LEFT)
    targetRollRate = -ROLL_RATE;

  // Pitch input
  if (xRotationDirection == Player::RX_DOWN)
    targetPitchRate = PITCH_RATE;
  else if (xRotationDirection == Player::RX_UP)
    targetPitchRate = -PITCH_RATE;

  // --- Bank-to-turn: yaw derived purely from roll angle ---
  // No direct yaw from A/D - roll first, then the bank produces the turn
  float speedFactor = speed / SPEED; // 1.0 at cruise speed
  float targetYawRate = -std::sin(transform.rotation.z) * BANK_TURN_FACTOR * speedFactor;

  // --- Smooth rotation rates (inertia) ---
  float rollLerp = 1.0f - std::exp(-ROLL_SMOOTHING * dt);
  float pitchLerp = 1.0f - std::exp(-PITCH_SMOOTHING * dt);
  float yawLerp = 1.0f - std::exp(-YAW_SMOOTHING * dt);

  currentRollRate += (targetRollRate - currentRollRate) * rollLerp;
  currentPitchRate += (targetPitchRate - currentPitchRate) * pitchLerp;
  currentYawRate += (targetYawRate - currentYawRate) * yawLerp;

  // --- Apply roll ---
  transform.rotation.z += currentRollRate * dt;

  // Auto-level roll when no A/D input
  if (yRotationDirection == Player::RY_NONE) {
    if (transform.rotation.z > 0.0f) {
      transform.rotation.z -= ROLL_RETURN_RATE * dt;
      if (transform.rotation.z < 0.0f) transform.rotation.z = 0.0f;
    } else if (transform.rotation.z < 0.0f) {
      transform.rotation.z += ROLL_RETURN_RATE * dt;
      if (transform.rotation.z > 0.0f) transform.rotation.z = 0.0f;
    }
  }

  // Clamp roll
  transform.rotation.z = std::max(-MAX_ROLL_ANGLE, std::min(MAX_ROLL_ANGLE, transform.rotation.z));

  // --- Apply pitch ---
  transform.rotation.x += currentPitchRate * dt;

  // Auto-level pitch gently when no W/S input
  if (xRotationDirection == Player::RX_NONE) {
    if (transform.rotation.x > 0.0f) {
      transform.rotation.x -= PITCH_RETURN_RATE * dt;
      if (transform.rotation.x < 0.0f) transform.rotation.x = 0.0f;
    } else if (transform.rotation.x < 0.0f) {
      transform.rotation.x += PITCH_RETURN_RATE * dt;
      if (transform.rotation.x > 0.0f) transform.rotation.x = 0.0f;
    }
  }

  // Clamp pitch
  transform.rotation.x = std::max(-MAX_PITCH_ANGLE, std::min(MAX_PITCH_ANGLE, transform.rotation.x));

  // --- Apply yaw (derived from bank) ---
  transform.rotation.y += currentYawRate * dt;

  // --- Speed: throttle + pitch-based speed transfer ---
  // Throttle (same keys: Shift accelerate, Ctrl decelerate)
  if (window.keyIsHeld(window.getKeyState(SDLK_LSHIFT)))
    speed += ACCELERATION * dt;
  else if (window.getScrollSpeed() > 0.0)
    speed += ACCELERATION * 4.0f * dt;
  else if (window.keyIsHeld(window.getKeyState(SDLK_LCTRL)))
    speed -= ACCELERATION * dt;
  else if (window.getScrollSpeed() < 0.0)
    speed -= ACCELERATION * 4.0f * dt;

  // Diving speeds up, climbing slows down
  speed += std::sin(transform.rotation.x) * PITCH_SPEED_TRANSFER * dt;

  // Clamp speed
  speed = std::max(SPEED * 0.5f, std::min(SPEED * 3.0f, speed));

  // --- Gravity and Lift ---
  // Lift depends on speed and how level the wings are (cos of roll)
  float liftAccel = LIFT_FACTOR * GRAVITY * (speed / SPEED) * std::cos(transform.rotation.z);
  // Net vertical acceleration: lift - gravity
  float netVertAccel = liftAccel - GRAVITY;
  verticalVelocity += netVertAccel * dt;
  // Dampen vertical velocity to prevent wild oscillation
  verticalVelocity *= std::exp(-3.0f * dt);

  // --- Position integration ---
  transform.position += transform.direction() * speed * dt;
  transform.position.y += verticalVelocity * dt;

  // Safety floor
  if (transform.position.y < MIN_HEIGHT) {
    transform.position.y = MIN_HEIGHT;
    if (verticalVelocity < 0.0f)
      verticalVelocity = 0.0f;
  }
}

void Player::resetShootTimer() { shoottimer = 0.2f; }

void Player::checkIfCrashed(float dt, const infworld::worldseed &permutations) {
  if (crashed)
    return;

  // Check if we ran out of health
  if (health <= 0) {
    crashed = true;
    return;
  }

  glm::vec3 pos = transform.position + transform.direction() * SPEED * dt;
  float h = infworld::getHeight(pos.z / SCALE * float(PREC + 1) / float(PREC),
                                pos.x / SCALE * float(PREC + 1) / float(PREC),
                                permutations) *
            HEIGHT * SCALE;
  // Maximum height difference between
  const float MAX_HEIGHT_DIFF = 8.0f;
  if (pos.y - h < MAX_HEIGHT_DIFF || pos.y < MAX_HEIGHT_DIFF / 2.0f) {
    crashed = true;
    return;
  }

  glm::vec3 positions[] = {
      transform.position + transform.rotate(glm::vec3(-9.0f, 0.0f, 0.0f)),
      transform.position + transform.rotate(glm::vec3(10.0f, 0.0f, 0.0f)),
      transform.position + transform.rotate(glm::vec3(-13.0f, -5.0f, 0.0f)),
      transform.position + transform.rotate(glm::vec3(13.0f, -5.0f, 0.0f)),
  };

  for (int i = 0; i < 4; i++) {
    glm::vec3 pos = positions[i];
    h = infworld::getHeight(pos.z / SCALE * float(PREC + 1) / float(PREC),
                            pos.x / SCALE * float(PREC + 1) / float(PREC),
                            permutations) *
        HEIGHT * SCALE;
    if (pos.y - h < MAX_HEIGHT_DIFF || pos.y < MAX_HEIGHT_DIFF / 2.0f) {
      crashed = true;
      return;
    }
  }
}
} // namespace gameobjects
