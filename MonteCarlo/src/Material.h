#pragma once
#include<glm.hpp>

class Material {
public:
    virtual ~Material() = default;

    bool isLight ;
    float lightIntensity;
    Vec3 color;
    double specularRate = 0;

    Material(const glm::vec3& color):color(color) {
        isLight = false;
        lightIntensity = 1.0f;
    }

    Material(const glm::vec3& color,double spec) :color(color) ,specularRate(spec){
        isLight = false;
        lightIntensity = 1.0f;
    }
    
};

auto RED = std::make_shared<Material>(glm::vec3(1.0f, 0.2f, 0.2f));
auto WHITE = std::make_shared<Material>(glm::vec3(1));
auto GRAY = std::make_shared<Material>(glm::vec3(0.6));
auto CYAN = std::make_shared<Material>(glm::vec3(0.1f,1.0f,1.0f));
auto BLUE = std::make_shared<Material>(glm::vec3(0, 0, 1.0f));
auto YELLOW= std::make_shared<Material>(glm::vec3(1.0f, 1.0f, 0));
auto GREEN = std::make_shared<Material>(glm::vec3(0, 1.0f, 0));
auto GREEN_MIRROR = std::make_shared<Material>(glm::vec3(0, 1.0f, 0),0.8);