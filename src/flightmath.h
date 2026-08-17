#ifndef RIVERRAID3D_FLIGHTMATH_H
#define RIVERRAID3D_FLIGHTMATH_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

//Small pure helpers for guided flight. Kept free of engine dependencies so the
//guidance maths can be exercised on its own.
namespace flight {
    //Rotates 'current' toward 'wanted', turning at most 'maxStep' radians.
    //Both vectors are assumed normalised; the result is normalised.
    inline glm::vec3 steerToward(const glm::vec3 &current, const glm::vec3 &wanted, float maxStep) {
        const float cosangle = glm::clamp(glm::dot(current, wanted), -1.0f, 1.0f);
        const float angle = std::acos(cosangle);

        if (angle <= maxStep)
            return glm::normalize(wanted);

        const glm::vec3 axis = glm::cross(current, wanted);
        const float axislen = glm::length(axis);
        if (axislen < 0.0001f) {
            //Exactly aligned or exactly reversed. Aligned needs no turn, and
            //reversed has no unique turn direction, so hold the current
            //heading rather than picking one arbitrarily.
            return glm::normalize(current);
        }

        const glm::vec4 turned =
                glm::rotate(glm::mat4(1.0f), maxStep, axis / axislen) * glm::vec4(current, 0.0f);
        return glm::normalize(glm::vec3(turned));
    }

    //Yaw and pitch that make game::Transform::direction() return 'heading'.
    //Transform builds its direction by rotating (0,0,1) about Y, then X, then
    //Z, which works out to (cos(p)sin(y), -sin(p), cos(p)cos(y)); this inverts
    //that. Returns (pitch, yaw), matching Transform::rotation.x and .y.
    inline glm::vec2 headingToEuler(const glm::vec3 &heading) {
        const glm::vec3 h = glm::normalize(heading);
        const float yaw = std::atan2(h.x, h.z);
        const float pitch = -std::asin(glm::clamp(h.y, -1.0f, 1.0f));
        return {pitch, yaw};
    }
}

#endif //RIVERRAID3D_FLIGHTMATH_H
