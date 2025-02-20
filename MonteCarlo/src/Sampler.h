#pragma once
#include<glm.hpp>
#include <gtc/constants.hpp>
#include <random>

class Sampler {
public:
    virtual ~Sampler() = default;

    // ���ɶ�ά���� [0,1)^2
    virtual glm::vec2 sample2D() { return glm::vec2(1.0); };

    // ���ɰ����򣨻��ھֲ�����ϵ�ķ��ߣ�
    virtual glm::vec3 sampleHemisphere(const glm::vec3& normal) {
        glm::vec2 uv = sample2D();
        // Ĭ�Ͼ��Ȱ������
        float z = uv.x;
        float r = glm::sqrt(1.0f - z * z);
        float phi = 2.0f * glm::pi<float>() * uv.y;
        glm::vec3 dir(r * glm::cos(phi), r * glm::sin(phi), z);
        return alignToNormal(dir, normal);
    }

protected:
    // ���ֲ�������뵽���߷���
    glm::vec3 alignToNormal(glm::vec3 dir, const glm::vec3& normal) {
        glm::vec3 up = abs(normal.z) < 0.999f ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
        glm::vec3 tangent = glm::normalize(glm::cross(up, normal));
        glm::vec3 bitangent = glm::cross(normal, tangent);
        return tangent * dir.x + bitangent * dir.y + normal * dir.z;
    }
};

//���Ȳ�����
class UniformSampler : public Sampler {
    std::mt19937 gen;
    std::uniform_real_distribution<float> dist;

public:
    UniformSampler(uint32_t seed = 0) : gen(seed), dist(0.0f, 1.0f) {}

    glm::vec2 sample2D() override {
        return { dist(gen), dist(gen) };
    }
};

//Cosine��Ȩ������
class CosineSampler : public Sampler {
public:
    glm::vec3 sampleHemisphere(const glm::vec3& normal) override {
        glm::vec2 uv = sample2D();

        // Cosine-weighted����
        float phi = 2 * glm::pi<float>() * uv.x;
        float r = glm::sqrt(uv.y);
        float x = r * glm::cos(phi);
        float y = r * glm::sin(phi);
        float z = glm::sqrt(1 - uv.y);

        return alignToNormal(glm::vec3(x, y, z), normal);
    }
};

//Halton�Ͳ�������
class HaltonSampler : public Sampler {
    uint32_t index;
    const int bases[2] = { 2, 3 }; // ���ʻ���

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