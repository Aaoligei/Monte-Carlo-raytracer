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
const int SAMPLE = 1024;

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

float angleBetweenVectors(const glm::vec3& a, const glm::vec3& b) {
    // 计算点积
    float dotProduct = glm::dot(a, b);

    // 计算向量长度
    float magnitudeA = glm::length(a);
    float magnitudeB = glm::length(b);

    // 避免除零错误（如果输入为零向量）
    if (magnitudeA < 1e-6 || magnitudeB < 1e-6) {
        return 0.0f; // 或抛出异常，根据需求处理
    }

    // 计算夹角的余弦值
    float cosTheta = dotProduct / (magnitudeA * magnitudeB);

    // 使用反余弦获取弧度值，并转换为角度
    float angleRadians = glm::acos(cosTheta);
    float angleDegrees = glm::degrees(angleRadians);

    return angleDegrees; // 返回角度值（0°~180°）
}

// 光线颜色计算（包括碰撞计算）
Vec3 ray_color(const Ray& ray, const Scene& scene, const Camera& cam) {
    HitRecord rec;
    float depth = 1.0f;
    //光线命中
    if (scene.hit(ray, 0.0f, FLT_MAX, rec)) {
        glm::vec3 lk = cam.lookat;
        float depth = glm::distance(rec.point ,cam.position)*abs(cos(angleBetweenVectors(lk,glm::vec3(rec.point-cam.position))));

       return Vec3(glm::clamp(depth, 0.0f, 1.0f));
        //return 0.5f * (rec.normal + glm::vec3(1.0f));
    }

    // 背景渐变
    //glm::vec3 unit_dir = glm::normalize(ray.direction);
    //float t = 0.5f * (unit_dir.y + 1.0f);
    //return (1.0f - t) * Vec3(1.0f) + t * Vec3(0.5f, 0.7f, 1.0f);
    return Vec3(0);
}

Vec3 ray_color1(const Ray& ray, const Scene& scene, const Camera& cam) {
    HitRecord rec;
    // 使用相机的近平面和远平面作为光线步进范围
    if (scene.hit(ray, cam.zNear, cam.zFar, rec)) {
        // 计算视线方向（假设 cam.lookat 是相机朝向的目标点）
        glm::vec3 viewDir = glm::normalize(cam.lookat - cam.position);
        // 命中点到相机的向量
        glm::vec3 toPoint = rec.point - cam.position;
        // 沿视线方向的投影深度（线性深度）
        float depth = glm::dot(toPoint, viewDir);
        // 将深度线性插值到 [0,1]
        float normalizedDepth = (depth - cam.zNear) / (cam.zFar - cam.zNear);
        normalizedDepth = glm::clamp(normalizedDepth, 0.0f, 1.0f);
        // 返回灰度颜色（0为黑色，1为白色）
        return Vec3(1-10*normalizedDepth);
    }

    return Vec3(0);
}

// 修改ray_color函数
Vec3 ray_color2(const Ray& ray, const Scene& scene,
    const std::vector<PointLight>& lights)
{
    HitRecord rec;
    if (!scene.hit(ray, 0.001f, FLT_MAX, rec)) {
        return Vec3(0);
    }

    const auto mat = rec.material;
    glm::vec3 result(0.0f);

    for (const auto& light : lights) {
        glm::vec3 light_dir = glm::normalize(light.position - rec.point);
        float distance = glm::distance(light.position, rec.point);

        // 阴影检测
        Ray shadow_ray(rec.point + rec.normal * 0.001f, light_dir);
        if (scene.is_shadowed(shadow_ray, distance)) continue;

        // 光照计算
        result += mat->shade(light_dir,
            -ray.direction,
            rec.normal,
            light.color * light.intensity);
    }

    return result;
}

Vec3 Path_tracing(const Ray& ray, const Scene& scene) {
    //return Vec3(1.0f);
    HitRecord rec;
    if (!scene.hit_nobvh(ray, 0.001f, FLT_MAX, rec)) {
        return Vec3(0);
    }
    if (rec.material->isLight) {
        return Vec3(1.0f);
    }
    Ray randomRay(rec.point, randomDirection(rec.normal));
    //反射光检测
    if (!scene.hit_nobvh(randomRay, 0.001f, FLT_MAX, rec)) {
        return Vec3(0);
    }
    if(rec.material->isLight) {
        return Vec3(1.0f)*rec.material->color;
    }
    else {
        return Vec3(0);
    }
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
                std::make_shared<Lambertian>(glm::vec3(rand() % 1000 / 1000.0f))
            ));
        }
    }
    return objects;
}

// 主渲染函数
void render_scene() {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    std::vector<PointLight> lights = {
        PointLight(glm::vec3(2,2,2)),
        PointLight(glm::vec3(2,2,-2))// 光源位置
    };

    //面光
    Triangle l1 = Triangle(glm::vec3(0.6, 0.99, 0.4), glm::vec3(-0.2, 0.99, -0.4), glm::vec3(-0.2,
        0.99, 0.4), white_mat);
    l1.material->isLight = true;
    Triangle l2 = Triangle(glm::vec3(0.6, 0.99, 0.4), glm::vec3(0.6, 0.99, -0.4), glm::vec3(-0.2,
        0.99, -0.4), white_mat);
    l2.material->isLight = true;

    //ObjMesh obj = load_obj("square.obj");
    //Mesh mesh(obj);

    Model model("C:/Users/25342/OneDrive/桌面/Monte-Carlo-raytracer/MonteCarlo/obj/cube.obj");
    Mesh mesh(model,red_mat);
    
    // 场景设置
    Scene scene;
    Sphere sphere(glm::vec3(0, 0, -1), 0.5f,red_mat);
    Triangle tri(glm::vec3(-0.5, -0.5, -0.5), glm::vec3(0.5, -0.5, -0.5), glm::vec3(0, -0.5, 0.5),red_mat);
    Triangle tri1(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0),red_mat);
    Sphere ground(glm::vec3(0, -1000, 0), 999.0f, ground_mat);

    //auto objects = create_stress_scene(); // 生成121个球体
    std::vector<std::shared_ptr<Hittable>> objects;
    objects.push_back(std::make_shared<Sphere>(ground));
    //objects.push_back(std::make_shared<Mesh>(mesh));
    objects.push_back(std::make_shared<Triangle>(l1));
    objects.push_back(std::make_shared<Triangle>(l2));
    objects.push_back(std::make_shared<Triangle>(tri));
    scene.objects = objects;
    //scene.build_bvh(objects);

    //scene.add(std::make_shared<Sphere>(sphere));
    //scene.add(std::make_shared<Sphere>(ground));
    //scene.add(std::make_shared<Triangle>(tri));
    //scene.add(std::make_shared<Triangle>(tri1));
    //scene.add(std::make_shared<Sphere>(glm::vec3(0, -100.5, -1), 100.0f));
    //scene.add(std::make_shared<Mesh>(mesh));

    // 相机配置
    Camera cam(
        glm::vec3(0, 0, 4),
        glm::vec3(0, 0, -1),
        90.0f,
        float(WIDTH) / HEIGHT
    );

    std::vector<uint8_t> pixels(WIDTH * HEIGHT * 3);

    omp_set_num_threads(50);
#pragma omp parallel for
    // 逐像素渲染
    for (int k = 0; k < SAMPLE; k++) {
        for (int j = 0; j < HEIGHT; ++j) {
            for (int i = 0; i < WIDTH; ++i) {
                float u = float(i) / (WIDTH - 1);
                float v = float(HEIGHT - 1 - j) / (HEIGHT - 1);

                Ray ray = cam.shootRay(u, v);
                //Vec3 color = ray_color2(ray, scene,lights);
                Vec3 color = Path_tracing(ray, scene);

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