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

auto RED = std::make_shared<Material>(glm::vec3(1.0f, 0.2f, 0.2f));
auto WHITE = std::make_shared<Material>(glm::vec3(1));
auto CYAN = std::make_shared<Material>(glm::vec3(0.1f,1.0f,1.0f));
auto BLUE = std::make_shared<Material>(glm::vec3(0, 0, 1.0f));
auto YELLOW= std::make_shared<Material>(glm::vec3(1.0f, 1.0f, 0));