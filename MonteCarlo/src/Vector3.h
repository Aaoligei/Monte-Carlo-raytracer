#pragma once
#include <glm.hpp>
// 基础向量类
class Vec3 : public glm::vec3 {
public:
    using glm::vec3::vec3;
    Vec3(const glm::vec3& v) : glm::vec3(v) {}

    uint8_t r() const { return static_cast<uint8_t>(255 * glm::clamp(x, 0.0f, 1.0f)); }
    uint8_t g() const { return static_cast<uint8_t>(255 * glm::clamp(y, 0.0f, 1.0f)); }
    uint8_t b() const { return static_cast<uint8_t>(255 * glm::clamp(z, 0.0f, 1.0f)); }
};
