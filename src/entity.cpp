#include "entity.h"
#include "glm/glm.hpp"

namespace gameobjects {
    namespace {
        //Monotonic source of entity ids. Never reused, so a stale id can never
        //accidentally resolve to a different entity later in the run.
        unsigned int nextEntityId = 1;
    }

    DamageableEntity::DamageableEntity(glm::vec3 pos, int hp, unsigned int score)
    : hitpoints(hp), scorevalue(score), id(nextEntityId++)
    {
        transform.position = pos;
        transform.scale    = glm::vec3(1.0f);
        transform.rotation = glm::vec3(0.0f);
    }

    float DamageableEntity::getVal(const std::string& key) const
    {
        auto it = values.find(key);
        return it != values.end() ? it->second : 0.0f;
    }

    void DamageableEntity::setVal(const std::string& key, float v)
    {
        values[key] = v;
    }

    Balloon::Balloon(glm::vec3 pos)
        : DamageableEntity(pos, 5, 10) {}

    Ship::Ship(glm::vec3 pos)
        : DamageableEntity(pos, 10, 50) {}

    Plane::Plane(glm::vec3 pos, float rotation)
        : DamageableEntity(pos, 14, 50)
    {
        transform.rotation.y = rotation;
    }

    Barrel::Barrel(glm::vec3 pos)
        : DamageableEntity(pos, 3, 5) {}

}


