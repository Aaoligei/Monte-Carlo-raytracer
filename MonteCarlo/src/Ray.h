#pragma once
#include<glm.hpp>
// ������
class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(const glm::vec3& o, const glm::vec3& d)
        : origin(o), direction(glm::normalize(d)) {}

    glm::vec3 at(float t) const {
        return origin + t * direction;
    }
};