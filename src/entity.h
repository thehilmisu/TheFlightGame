#ifndef RIVERRAID3D_ENTITY_H
#define RIVERRAID3D_ENTITY_H

#include "transform.h"
#include <unordered_map>
#include <string>
#include <vector>
#include "infworld.h"

namespace gameobjects { struct Player; struct Bullet;}

struct UpdateContext {
    const gameobjects::Player *player = nullptr;
    std::vector<gameobjects::Bullet> *bullets = nullptr;
    const infworld::worldseed* worldseed = nullptr;
    float totalTime = 0.0f;
};

namespace gameobjects {
    class Entity {
    public:
        game::Transform transform;
        virtual void update(float dt, const UpdateContext& context) = 0;
        [[nodiscard]] virtual bool isDead() const = 0;
        virtual ~Entity() = default;
    };

    class DamageableEntity : public Entity {
    public:
        int hitpoints = 0;
        unsigned int scorevalue = 0;
        std::unordered_map<std::string, float> values;

        DamageableEntity(glm::vec3 pos, int hp, unsigned int score);
        [[nodiscard]] bool isDead() const override { return hitpoints <= 0; };
        [[nodiscard]] float getVal(const std::string &key) const;
        void setVal(const std::string &key, float v);
    };


    // ── Concrete enemy types ────────────────────────────────────────────────────
    class Balloon : public DamageableEntity {
    public:
        explicit Balloon(glm::vec3 pos);
        void update(float dt, const UpdateContext& ctx) override;
    };

    class Ship : public DamageableEntity {
    public:
        explicit Ship(glm::vec3 pos);
        void update(float dt, const UpdateContext& ctx) override;
    };

    class Plane : public DamageableEntity {
    public:
        Plane(glm::vec3 pos, float rotation);
        void update(float dt, const UpdateContext& ctx) override;
        void checkIfCrashed(const infworld::worldseed& permutations);
    };

    class Blimp : public DamageableEntity {
    public:
        Blimp(glm::vec3 pos, float rotation);
        void update(float dt, const UpdateContext& ctx) override;   // stub
    };

    class Ufo : public DamageableEntity {
    public:
        Ufo(glm::vec3 pos, float rotation);
        void update(float dt, const UpdateContext& ctx) override;   // stub
    };

    // ── Destructible props ──────────────────────────────────────────────────────
    class Barrel : public DamageableEntity {
    public:
        explicit Barrel(glm::vec3 pos);
        void update(float dt, const UpdateContext& ctx) override;
    };
}

#endif //RIVERRAID3D_ENTITY_H