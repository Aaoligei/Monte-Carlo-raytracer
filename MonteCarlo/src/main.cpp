#include <iostream>
#include <vector>
#include <glm.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>

#include"Vector3.h"
#include"Camera.h"
#include"Ray.h"
#include"HitRecord.h"
#include"obj_loader.h"

// 光线颜色计算（包括碰撞计算）
Vec3 ray_color(const Ray& ray, const Scene& scene) {
    HitRecord rec;
    if (scene.hit(ray, 0.0f, FLT_MAX, rec)) {
        return 0.5f * (rec.normal + glm::vec3(1.0f));
    }

    // 背景渐变
    glm::vec3 unit_dir = glm::normalize(ray.direction);
    float t = 0.5f * (unit_dir.y + 1.0f);
    return (1.0f - t) * Vec3(1.0f) + t * Vec3(0.5f, 0.7f, 1.0f);
}

void init_scene(Scene& scene) {
    // X轴（红色）
    scene.add(std::make_shared<Cylinder>(
        glm::vec3(-100, 0, 0),  // 起点
        glm::vec3(100, 0, 0),   // 终点
        0.1f,                 // 半径
        glm::vec3(1, 0, 0)      // 红色
    ));

    // Y轴（绿色）
    scene.add(std::make_shared<Cylinder>(
        glm::vec3(0, -100, 0),
        glm::vec3(0, 100, 0),
        0.1f,
        glm::vec3(0, 1, 0)
    ));

    // Z轴（蓝色）
    scene.add(std::make_shared<Cylinder>(
        glm::vec3(0, 0, -100),
        glm::vec3(0, 0, 100),
        0.1f,
        glm::vec3(0, 0, 1)
    ));

    // 添加坐标箭头（圆锥体，可选）
    // ...
}

// 主渲染函数
void render_scene() {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    ObjMesh obj = load_obj("square.obj");
    Mesh mesh(obj);
    Mesh testMesh(obj);
    testMesh.triangles.clear();
    
    // 场景设置
    Scene scene;
    //init_scene(scene);
    Sphere sphere(glm::vec3(0, 0, -1), 0.5f);
    Triangle tri(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0));
    testMesh.triangles.push_back(tri);

    scene.add(std::make_shared<Sphere>(sphere));
    //scene.add(std::make_shared<Triangle>(tri));
    //scene.add(std::make_shared<Sphere>(glm::vec3(0, -100.5, -1), 100.0f));
    scene.add(std::make_shared<Mesh>(testMesh));

    // 相机配置
    Camera cam(
        glm::vec3(0, 0, 3),
        glm::vec3(0, 0, -1),
        90.0f,
        float(WIDTH) / HEIGHT
    );

    std::vector<uint8_t> pixels(WIDTH * HEIGHT * 3);

    // 逐像素渲染
    for (int j = 0; j < HEIGHT; ++j) {
        for (int i = 0; i < WIDTH; ++i) {
            float u = float(i) / (WIDTH - 1);
            float v = float(HEIGHT - 1 - j) / (HEIGHT - 1);

            Ray ray = cam.generateRay(u, v);
            Vec3 color = ray_color(ray, scene);

            size_t index = 3 * (j * WIDTH + i);
            pixels[index] = color.r();
            pixels[index + 1] = color.g();
            pixels[index + 2] = color.b();
        }
    }

    stbi_write_png("render_output.png", WIDTH, HEIGHT, 3, pixels.data(), WIDTH * 3);
}

int main() {
    render_scene();
    std::cout << "Rendering completed!" << std::endl;
    return 0;
}