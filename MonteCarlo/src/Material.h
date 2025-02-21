#pragma once
#include<glm.hpp>
#include"Sampler.h"

class Material {
public:
    virtual ~Material() = default;

    bool isLight=false;
    Vec3 color;

    virtual glm::vec3 shade(const glm::vec3& light_dir,
        const glm::vec3& view_dir,
        const glm::vec3& normal,
        const glm::vec3& light_color) const = 0;

    virtual glm::vec3 sample(const glm::vec3& normal,
        Sampler& sampler,
        float& pdf) const
    {
        glm::vec3 dir = sampler.sampleHemisphere(normal);
        pdf = 1.0f / (2 * glm::pi<float>()); // 均匀采样的PDF
        return dir;
    }
};

class Lambertian : public Material {
    glm::vec3 albedo; // 漫反射颜色
    float kd;         // 漫反射系数

public:
    Lambertian(const glm::vec3& col, float diffuse = 1.0f)
        : albedo(col), kd(diffuse) {
        color = Vec3(albedo.x,albedo.y, albedo.z);
    }

    glm::vec3 shade(const glm::vec3& light_dir,
        const glm::vec3& view_dir,
        const glm::vec3& normal,
        const glm::vec3& light_color) const override
    {
        float cos_theta = glm::max(glm::dot(normal, light_dir), 0.0f);
        return kd * albedo * light_color * cos_theta;
    }

    glm::vec3 sample(const glm::vec3& normal,
        Sampler& sampler,
        float& pdf) const override
    {
        CosineSampler cs; // 强制使用Cosine采样
        glm::vec3 dir = cs.sampleHemisphere(normal);
        pdf = glm::dot(dir, normal) / glm::pi<float>(); // PDF = cosθ/π
        return dir;
    }
};

auto red_mat = std::make_shared<Lambertian>(glm::vec3(1.0f, 0.2f, 0.2f));
auto ground_mat = std::make_shared<Lambertian>(glm::vec3(0.8f));
auto white_mat = std::make_shared<Lambertian>(glm::vec3(1.0f));