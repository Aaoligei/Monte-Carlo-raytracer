#pragma once
#include "HitRecord.h"

auto RED = std::make_shared<Material>(glm::vec3(1.0f, 0.2f, 0.2f));
auto WHITE = std::make_shared<Material>(glm::vec3(1));
auto GRAY = std::make_shared<Material>(glm::vec3(0.6));
auto CYAN = std::make_shared<Material>(glm::vec3(0.1f,1.0f,1.0f));
auto BLUE = std::make_shared<Material>(glm::vec3(0, 0, 1.0f));
auto YELLOW= std::make_shared<Material>(glm::vec3(1.0f, 1.0f, 0));
auto GREEN = std::make_shared<Material>(glm::vec3(0, 1.0f, 0));
auto GREEN_MIRROR = std::make_shared<Material>(glm::vec3(0, 1.0f, 0),0.8);
auto WHITE_MIRROR = std::make_shared<Material>(glm::vec3(1),0.8);