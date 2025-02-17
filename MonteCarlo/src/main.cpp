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
#include"Model.h"   
#include"Light.h"

#include <gtc/constants.hpp> // 包含π常量

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

// 主渲染函数
void render_scene() {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    std::vector<PointLight> lights = {
        PointLight(glm::vec3(2,2,2))// 光源位置
    };

    //ObjMesh obj = load_obj("square.obj");
    //Mesh mesh(obj);

    Model model("C:/Users/25342/OneDrive/桌面/Monte-Carlo-raytracer/MonteCarlo/src/cube.obj");
    Mesh mesh(model,red_mat);
    
    // 场景设置
    Scene scene;
    Sphere sphere(glm::vec3(0, 0, -1), 0.5f,red_mat);
    Triangle tri(glm::vec3(1, 1, 1), glm::vec3(-1, 1, 1), glm::vec3(-1, 1, -1),red_mat);
    Triangle tri1(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0),red_mat);
    Sphere ground(glm::vec3(0, -100.5, -1), 100.0f, ground_mat);

    //scene.add(std::make_shared<Sphere>(sphere));
    scene.add(std::make_shared<Sphere>(ground));
    //scene.add(std::make_shared<Triangle>(tri));
    //scene.add(std::make_shared<Triangle>(tri1));
    //scene.add(std::make_shared<Sphere>(glm::vec3(0, -100.5, -1), 100.0f));
    scene.add(std::make_shared<Mesh>(mesh));

    // 相机配置
    Camera cam(
        glm::vec3(2, 2, 3),
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
            Vec3 color = ray_color2(ray, scene,lights);

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