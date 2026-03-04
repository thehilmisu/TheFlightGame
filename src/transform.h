#ifndef RIVERRAID3D_TRANSFORM_H
#define RIVERRAID3D_TRANSFORM_H

#include "glm/glm.hpp"

namespace game {
    struct Transform {
        glm::vec3 position{};
        glm::vec3 scale{};
        glm::vec3 rotation{};
        Transform();

        [[nodiscard]] glm::mat4 getTransformMat() const;

        [[nodiscard]] glm::vec3 direction() const;
        [[nodiscard]] glm::vec3 right() const;

        [[nodiscard]] glm::vec3 rotate(const glm::vec3 &v) const;

        [[nodiscard]] glm::vec3 invRotate(const glm::vec3 &v) const;
    };
}

#endif //RIVERRAID3D_TRANSFORM_H