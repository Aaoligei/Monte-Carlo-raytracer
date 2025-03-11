#pragma once
#include<glm.hpp>
// 光线类
class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(const glm::vec3& o, const glm::vec3& d)
        : origin(o), direction(glm::normalize(d)) {}

    glm::vec3 at(float t) const {
        return origin + t * direction;
    }

    // 添加射线与AABB检测函数（示例基于Slab算法）
    inline bool hitAABB(const glm::vec3& AA, const glm::vec3& BB, float t_min, float t_max) const {
        for (int i = 0; i < 3; ++i) {
            float invD = 1.0f / direction[i];
            float t0 = (AA[i] - origin[i]) * invD;
            float t1 = (BB[i] - origin[i]) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            t_min = std::max(t0, t_min);
            t_max = std::min(t1, t_max);
            if (t_max <= t_min) return false;
        }
        return true;
    }

};