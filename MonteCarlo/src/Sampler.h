#pragma once
#include<glm.hpp>
#include <gtc/constants.hpp>
#include <random>

class Sampler {
public:
    virtual ~Sampler() = default;

    // 生成二维样本 [0,1)^2
    virtual glm::vec2 sample2D() { return glm::vec2(1.0); };

    // 生成半球方向（基于局部坐标系的法线）
    virtual glm::vec3 sampleHemisphere(const glm::vec3& normal) {
        glm::vec2 uv = sample2D();
        // 默认均匀半球采样
        float z = uv.x;
        float r = glm::sqrt(1.0f - z * z);
        float phi = 2.0f * glm::pi<float>() * uv.y;
        glm::vec3 dir(r * glm::cos(phi), r * glm::sin(phi), z);
        return alignToNormal(dir, normal);
    }

protected:
    // 将局部方向对齐到法线方向
    glm::vec3 alignToNormal(glm::vec3 dir, const glm::vec3& normal) {
        glm::vec3 up = abs(normal.z) < 0.999f ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
        glm::vec3 tangent = glm::normalize(glm::cross(up, normal));
        glm::vec3 bitangent = glm::cross(normal, tangent);
        return tangent * dir.x + bitangent * dir.y + normal * dir.z;
    }
};

//均匀采样器
class UniformSampler : public Sampler {
    std::mt19937 gen;
    std::uniform_real_distribution<float> dist;

public:
    UniformSampler(uint32_t seed = 0) : gen(seed), dist(0.0f, 1.0f) {}

    glm::vec2 sample2D() override {
        return { dist(gen), dist(gen) };
    }
};

//Cosine加权采样器
class CosineSampler : public Sampler {
public:
    glm::vec3 sampleHemisphere(const glm::vec3& normal) override {
        glm::vec2 uv = sample2D();

        // Cosine-weighted采样
        float phi = 2 * glm::pi<float>() * uv.x;
        float r = glm::sqrt(uv.y);
        float x = r * glm::cos(phi);
        float y = r * glm::sin(phi);
        float z = glm::sqrt(1 - uv.y);

        return alignToNormal(glm::vec3(x, y, z), normal);
    }
};

//Halton低差异序列
class HaltonSampler : public Sampler {
    uint32_t index;
    const int bases[2] = { 2, 3 }; // 互质基数

public:
    HaltonSampler(uint32_t start = 0) : index(start) {}

    glm::vec2 sample2D() override {
        return { radicalInverse(bases[0], index),
                radicalInverse(bases[1], index++) };
    }

private:
    float radicalInverse(int base, uint32_t i) {
        float invBase = 1.0f / base;
        float result = 0.0f;
        float factor = invBase;
        while (i > 0) {
            result += (i % base) * factor;
            i /= base;
            factor *= invBase;
        }
        return result;
    }
};