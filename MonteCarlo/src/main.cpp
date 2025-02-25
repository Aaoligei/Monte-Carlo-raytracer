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
const int SAMPLE = 16;
const double BRIGHTNESS = (2.0f * 3.1415926f) * (1.0f / double(SAMPLE));
const int WIDTH = 800;
const int HEIGHT = 600;

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

Vec3 PathTracing(const Ray& ray, const Scene& scene) {
    HitRecord rec;
    if (!scene.hit(ray, 0.001f, FLT_MAX, rec)) {
        return Vec3(0);
    }
    if (rec.material->isLight) {
        return Vec3(1.0f);
    }
    else {
        return Vec3(0);
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
    Vec3 ptColor = PathTracing(randomRay, scene);
    Vec3 color = srcColor * ptColor;
    return Vec3(color.x * BRIGHTNESS, color.y * BRIGHTNESS, color.z * BRIGHTNESS);
}



// 生成随机球体场景
std::vector<std::shared_ptr<Hittable>> create_stress_scene() {
    std::vector<std::shared_ptr<Hittable>> objects;
    for (int a = -5; a <= 5; ++a) {
        for (int b = -5; b <= 5; ++b) {
            glm::vec3 center(a + 0.9f * rand() / RAND_MAX,
                0.2f,
                b + 0.9f * rand() / RAND_MAX);
            objects.push_back(std::make_shared<Sphere>(
                center, 0.2f,
                std::make_shared<Material>(glm::vec3(rand() % 1000 / 1000.0f))
            ));
        }
    }
    return objects;
}

// 主渲染函数
void render_scene() {


    std::vector<PointLight> lights = {
        PointLight(glm::vec3(2,2,2)),
        PointLight(glm::vec3(2,2,-2))// 光源位置
    };

    //面光
    Triangle l1 = Triangle(glm::vec3(0.6, 1.99, 0.4), glm::vec3(-0.2, 1.99, -0.4), glm::vec3(-0.2,
        1.99, 0.4), white_mat);
    l1.material->isLight = true;
    Triangle l2 = Triangle(glm::vec3(0.6, 1.99, 0.4), glm::vec3(0.6, 1.99, -0.4), glm::vec3(-0.2,
        1.99, -0.4), white_mat);
    l2.material->isLight = true;

    Model model("C:/Users/25342/OneDrive/桌面/Monte-Carlo-raytracer/MonteCarlo/obj/cube.obj");
    Mesh mesh(model,red_mat);
    
    // 场景设置
    Scene scene;
    Sphere sphere(glm::vec3(0, 0, -1), 0.5f,red_mat);
    Triangle tri(glm::vec3(0.5, -0.5, -0.5), glm::vec3(-0.5, -0.5, -0.5), glm::vec3(0, -0.5, 0.5),red_mat);
    Sphere ground(glm::vec3(0, -1000, 0), 999.0f, ground_mat);

    //auto objects = create_stress_scene(); // 生成121个球体
    scene.add(std::make_shared<Sphere>(ground));
    //objects.push_back(std::make_shared<Mesh>(mesh));
    scene.add(std::make_shared<Triangle>(l1));
    scene.add(std::make_shared<Triangle>(l2));
    //scene.add(std::make_shared<Triangle>(tri));
    //scene.add(std::make_shared<Mesh>(mesh));
    //scene.add(std::make_shared<Sphere>(sphere));

    // 相机配置
    Camera cam(
        glm::vec3(0, 0, 4),
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