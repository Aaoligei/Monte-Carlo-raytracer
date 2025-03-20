#pragma once
#include "HitRecord.h"
#include "Material.h"
#include <memory>

class CornellBox {
public:
    static void setup(Scene& scene) {
        // 定义Cornell box的材质
        auto white = std::make_shared<Material>(glm::vec3(0.73f, 0.73f, 0.73f));
        auto red = std::make_shared<Material>(glm::vec3(0.65f, 0.05f, 0.05f));
        auto green = std::make_shared<Material>(glm::vec3(0.12f, 0.45f, 0.15f));
        auto light = std::make_shared<Material>(glm::vec3(1.0f, 1.0f, 1.0f));
        light->isLight = true;

        // 顶部光源（缩放到-1到1的范围）
        glm::vec3 offset(0, 0, -0.2);
        scene.add(std::make_shared<Triangle>(glm::vec3(-0.2, 0.99, -0.2)+offset, glm::vec3(0.2, 0.99, 0.2) + offset, glm::vec3(-0.2,
            0.99, 0.2) + offset, light));
        scene.add(std::make_shared<Triangle>(glm::vec3(0.2, 0.99, 0.2) + offset, glm::vec3(0.2, 0.99, -0.2) + offset, glm::vec3(-0.2,
            0.99, -0.2) + offset, light));
        
        // bottom
        scene.add(std::make_shared<Triangle>(glm::vec3(1, -1, 1), glm::vec3(-1, -1, -1), glm::vec3(-1, -1, 1), white));
        scene.add(std::make_shared<Triangle>(glm::vec3(1, -1, 1), glm::vec3(1, -1, -1), glm::vec3(-1, -1, -1), white));
        // top
        scene.add(std::make_shared<Triangle>(glm::vec3(1, 1, 1), glm::vec3(-1, 1, 1), glm::vec3(-1, 1, -1), white));
        scene.add(std::make_shared<Triangle>(glm::vec3(1, 1, 1), glm::vec3(-1, 1, -1), glm::vec3(1, 1, -1), white));
        // back
        scene.add(std::make_shared<Triangle>(glm::vec3(1, -1, -1), glm::vec3(-1, 1, -1), glm::vec3(-1, -1, -1), white));
        scene.add(std::make_shared<Triangle>(glm::vec3(1, -1, -1), glm::vec3(1, 1, -1), glm::vec3(-1, 1, -1), white));
        // left
        scene.add(std::make_shared<Triangle>(glm::vec3(-1, -1, -1), glm::vec3(-1, 1, 1), glm::vec3(-1, -1, 1), green));
        scene.add(std::make_shared<Triangle>(glm::vec3(-1, -1, -1), glm::vec3(-1, 1, -1), glm::vec3(-1, 1, 1), green));
        // right
        scene.add(std::make_shared<Triangle>(glm::vec3(1, 1, 1), glm::vec3(1, -1, -1), glm::vec3(1, -1, 1), red));
        scene.add(std::make_shared<Triangle>(glm::vec3(1, -1, -1), glm::vec3(1, 1, 1), glm::vec3(1, 1, -1), red));
       
        ////box1
        //// bottom
        //scene.add(std::make_shared<Triangle>( glm::vec3(-0.5, -1, -0.5), glm::vec3(0.5, -1, 0), glm::vec3(-0.5, -1, 0), white));
        //scene.add(std::make_shared<Triangle>( glm::vec3(0.5, -1, -0.5), glm::vec3(0.5, -1, 0), glm::vec3(-0.5, -1, -0.5), white));
        //// top
        //scene.add(std::make_shared<Triangle>( glm::vec3(-0.5, 0, 0), glm::vec3(0.5, 0, 0), glm::vec3(-0.5, 0, -0.5), white));
        //scene.add(std::make_shared<Triangle>( glm::vec3(-0.5, 0, -0.5), glm::vec3(0.5, 0, 0), glm::vec3(0.5, 0, -0.5), white));
        //// back
        //scene.add(std::make_shared<Triangle>( glm::vec3(-0.5, 0, -0.5), glm::vec3(0.5, -1, -0.5), glm::vec3(-0.5, -1, -0.5), white));
        //scene.add(std::make_shared<Triangle>(glm::vec3(0.5, 0, -0.5), glm::vec3(0.5, -1, -0.5), glm::vec3(-0.5, 0, -0.5), white));
        //// left
        //scene.add(std::make_shared<Triangle>( glm::vec3(-0.5, 0, 0), glm::vec3(-0.5, -1, -0.5), glm::vec3(-0.5, -1, 0), white));
        //scene.add(std::make_shared<Triangle>( glm::vec3(-0.5, 0, -0.5), glm::vec3(-0.5, -1, -0.5), glm::vec3(-0.5, 0, 0), white));
        //// right
        //scene.add(std::make_shared<Triangle>(glm::vec3(0.5, -1, -0.5), glm::vec3(0.5, 0, 0), glm::vec3(0.5, -1, 0), white));
        //scene.add(std::make_shared<Triangle>( glm::vec3(0.5, 0, 0), glm::vec3(0.5, -1, -0.5), glm::vec3(0.5, 0, -0.5), white));
        ////front
        //scene.add(std::make_shared<Triangle>( glm::vec3(0.5, -1, 0), glm::vec3(-0.5, 0, 0), glm::vec3(-0.5, -1, 0), white));
        //scene.add(std::make_shared<Triangle>(glm::vec3(0.5, -1, 0), glm::vec3(0.5, 0, 0), glm::vec3(-0.5, 0, 0), white));
    
		//box2
		Box box1(0.5f,0.5f,0.5f, glm::vec3(0.3, -0.75, 0), white);
        Box box2(0.5f, 1.0f, 0.5f, glm::vec3(-0.3, -0.5, -0.4), white);
		box1.rotate(-20, glm::vec3(0, 1, 0));
        box2.rotate(20, glm::vec3(0, 1, 0));
		scene.add(std::make_shared<Box>(box1));
        scene.add(std::make_shared<Box>(box2));
    }
};