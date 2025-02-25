#pragma once
#include<glm.hpp>

class Material {
public:
    virtual ~Material() = default;
    bool isLight ;
    Vec3 color;

    Material(const glm::vec3& color):color(color) {
        isLight = false;
    }
    
};

auto red_mat = std::make_shared<Material>(glm::vec3(1.0f, 0.2f, 0.2f));
auto ground_mat = std::make_shared<Material>(glm::vec3(0.8f));
auto white_mat = std::make_shared<Material>(glm::vec3(1.0f));