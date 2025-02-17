#pragma once
// light.h
#include "glm.hpp"

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;

    PointLight(const glm::vec3& pos,
        const glm::vec3& col = glm::vec3(1.0f),
        float power = 1.0f)
        : position(pos), color(col), intensity(power) {}
};