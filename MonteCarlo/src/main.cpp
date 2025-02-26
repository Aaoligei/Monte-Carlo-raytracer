#include <iostream>
#include <vector>
#include <glm.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>
#include<omp.h>
#include<random>
#include<stdlib.h>
#include"svcpng.inc"

#include"Vector3.h"
#include"Camera.h"
#include"Ray.h"
#include"HitRecord.h"
#include"obj_loader.h"
#include"Model.h"   
#include"Light.h"
#include"Sampler.h"

#include <gtc/constants.hpp> // 包含π常量
#include <chrono>
const int SAMPLE = 128;
const double BRIGHTNESS = (2.0f * 3.1415926f) * (1.0f / double(SAMPLE));
const int WIDTH = 512;
const int HEIGHT = 512;

// 0-1 随机数生成
std::uniform_real_distribution<> dis(0.0, 1.0);
std::random_device rd;
std::mt19937 gen(rd());
double randf()
{
    return dis(gen);
}

// 单位球内的随机向量
glm::vec3 randomVec3()
{

    glm::vec3 d;
    do
    {
        d = 2.0f * glm::vec3(randf(), randf(), randf()) - glm::vec3(1, 1, 1);
    } while (dot(d, d) > 1.0);
    return glm::normalize(d);
    /*
    double r1 = randf(), r2 = randf();
    double z = sqrt(1.0f - r2);
    double phi = 2 * 3.1415926 * r1;
    float x = cos(phi) * sqrt(r2);
    float y = sin(phi) * sqrt(r2);
    return glm::normalize(glm::vec3(x, y, z));
    */
}

// 法向半球随机向量
glm::vec3 randomDirection(glm::vec3 n)
{
    /*
    // 法向半球
    glm::vec3 d;
    do
    {
        d = randomVec3();
    } while (glm::dot(d, n) < 0.0f);
    return d;
    */
    // 法向球
    return glm::normalize(randomVec3() + n);
}

Vec3 PathTracing(const Ray& ray, const Scene& scene ,int depth) {
    if (depth > 8) return glm::vec3(0);

    HitRecord rec;
    if (!scene.hit(ray, 0.001f, FLT_MAX, rec)) {
        return Vec3(0);
    }
    if (rec.material->isLight) {
        return Vec3(1.0f);
    }
    else {
        // 有 P 的概率终止
        double r = randf();
        float P = 0.8;
        if (r > P) return glm::vec3(0);

        Ray randomRay(rec.point, randomDirection(rec.normal));
        //反射光检测
        float cosine = glm::dot(-ray.direction, rec.normal);
        Vec3 srcColor = rec.material->color;
        Vec3 ptColor = PathTracing(randomRay, scene, depth+1)*cosine;
        Vec3 color = srcColor * ptColor;
        return color/P;
    }
}

Vec3 CalColor(const Ray& ray, const Scene& scene) {
    HitRecord rec;
    if (!scene.hit(ray, 0.001f, FLT_MAX, rec)) {
        return Vec3(0);
    }
    if (rec.material->isLight) {
        return Vec3(1.0f);
    }
    Ray randomRay(rec.point, randomDirection(rec.normal));
    //反射光检测
    Vec3 srcColor = rec.material->color;
    Vec3 ptColor = PathTracing(randomRay, scene,0);
    Vec3 color = srcColor * ptColor;
    return Vec3(color.x * BRIGHTNESS, color.y * BRIGHTNESS, color.z * BRIGHTNESS);
}

// 主渲染函数
void render_scene() {


    std::vector<PointLight> lights = {
        PointLight(glm::vec3(2,2,2)),
        PointLight(glm::vec3(2,2,-2))// 光源位置
    };

    //面光
    Triangle l1 = Triangle( glm::vec3(-0.4, 0.9, -0.4), glm::vec3(0.4, 0.9, 0.4), glm::vec3(-0.4,
        0.9, 0.4), WHITE);
    l1.material->isLight = true;
    Triangle l2 = Triangle(glm::vec3(0.4, 0.9, 0.4), glm::vec3(0.4, 0.9, -0.4), glm::vec3(-0.4,
        0.9, -0.4), WHITE);
    l2.material->isLight = true;

    Model model("C:/Users/25342/OneDrive/桌面/Monte-Carlo-raytracer/MonteCarlo/obj/cube.obj");
    Mesh mesh(model,RED);
    
    // 场景设置
    Scene scene;
    Sphere sphere(glm::vec3(0, 0, -1), 0.5f, RED);
    Triangle tri(glm::vec3(-0.5, -0.5, -0.5),glm::vec3(0.5, -0.5, -0.5), glm::vec3(0, -0.5, 0.5), RED);

    scene.add(std::make_shared<Triangle>(l1));
    scene.add(std::make_shared<Triangle>(l2));

    scene.add(std::make_shared<Triangle>(glm::vec3(-0.15, 0.4, -0.6), glm::vec3(-0.15, -0.95, -0.6), glm::vec3(0.15, 0.4, -0.6), YELLOW));
    scene.add(std::make_shared<Triangle>(glm::vec3(0.15, 0.4, -0.6), glm::vec3(-0.15, -0.95, -0.6), glm::vec3(0.15, -0.95, -0.6), YELLOW));
    // 背景盒子
    // bottom
    scene.add(std::make_shared<Triangle>(glm::vec3(1, -1, 1), glm::vec3(-1, -1, -1), glm::vec3(-1, -1, 1), RED));
    scene.add(std::make_shared<Triangle>( glm::vec3(1, -1, 1), glm::vec3(1, -1, -1), glm::vec3(-1, -1, -1), RED));
    // top
    scene.add(std::make_shared<Triangle>(glm::vec3(1, 1, 1), glm::vec3(-1, 1, 1), glm::vec3(-1, 1, -1), RED));
    scene.add(std::make_shared<Triangle>(glm::vec3(1, 1, 1), glm::vec3(-1, 1, -1), glm::vec3(1, 1, -1), RED));
    // back
    scene.add(std::make_shared<Triangle>(glm::vec3(1, -1, -1), glm::vec3(-1, 1, -1), glm::vec3(-1, -1, -1), CYAN));
    scene.add(std::make_shared<Triangle>(glm::vec3(1, -1, -1), glm::vec3(1, 1, -1), glm::vec3(-1, 1, -1), CYAN));
    // left
    scene.add(std::make_shared<Triangle>(glm::vec3(-1, -1, -1), glm::vec3(-1, 1, 1), glm::vec3(-1, -1, 1), BLUE));
    scene.add(std::make_shared<Triangle>(glm::vec3(-1, -1, -1), glm::vec3(-1, 1, -1), glm::vec3(-1, 1, 1), BLUE));
    // right
    scene.add(std::make_shared<Triangle>(glm::vec3(1, 1, 1), glm::vec3(1, -1, -1), glm::vec3(1, -1, 1), RED));
    scene.add(std::make_shared<Triangle>(glm::vec3(1, -1, -1), glm::vec3(1, 1, 1), glm::vec3(1, 1, -1), RED));

    // 相机配置
    Camera cam(
        glm::vec3(0, 0, 2),
        glm::vec3(0, 0, -1),
        90.0f,
        float(WIDTH) / HEIGHT
    );

    std::vector<uint8_t> pixels(WIDTH * HEIGHT * 3);

    double* image = new double[WIDTH * HEIGHT * 3];
    memset(image, 0.0, sizeof(double) * WIDTH * HEIGHT * 3);
    omp_set_num_threads(50);
#pragma omp parallel for
    // 逐像素渲染
   for (int k = 0; k < SAMPLE; k++) {
        for (int j = 0; j < HEIGHT; ++j) {
            for (int i = 0; i < WIDTH; ++i) {
                float u = float(i) / (WIDTH - 1);
                float v = float(HEIGHT - 1 - j) / (HEIGHT - 1);

                Ray ray = cam.shootRay(u, v);
                Vec3 color = CalColor(ray, scene);

                size_t index = 3 * (j * WIDTH + i);
                pixels[index] += color.r();
                pixels[index + 1] += color.g();
                pixels[index + 2] += color.b();
            }
        }
    }

    stbi_write_png("render_output.png", WIDTH, HEIGHT, 3, pixels.data(), WIDTH * 3);

}

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    render_scene();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Rendering completed!" << std::endl;
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Render time: " << duration.count() << "ms\n";
    return 0;
}