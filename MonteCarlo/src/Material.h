#pragma once
#include<glm.hpp>

class Material {
public:
    virtual ~Material() = default;

    virtual glm::vec3 shade(const glm::vec3& light_dir,
        const glm::vec3& view_dir,
        const glm::vec3& normal,
        const glm::vec3& light_color) const = 0;
};

class Lambertian : public Material {
    glm::vec3 albedo; // 漫反射颜色
    float kd;         // 漫反射系数

public:
    Lambertian(const glm::vec3& color, float diffuse = 1.0f)
        : albedo(color), kd(diffuse) {}

    glm::vec3 shade(const glm::vec3& light_dir,
        const glm::vec3& view_dir,
        const glm::vec3& normal,
        const glm::vec3& light_color) const override
    {
        float cos_theta = glm::max(glm::dot(normal, light_dir), 0.0f);
        return kd * albedo * light_color * cos_theta;
    }
};

auto red_mat = std::make_shared<Lambertian>(glm::vec3(1.0f, 0.2f, 0.2f));
auto ground_mat = std::make_shared<Lambertian>(glm::vec3(0.8f));