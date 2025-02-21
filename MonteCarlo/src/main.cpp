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

#include <gtc/constants.hpp> // �����г���
#include <chrono>
const int SAMPLE = 1024;

// 0-1 ���������
std::uniform_real_distribution<> dis(0.0, 1.0);
std::random_device rd;
std::mt19937 gen(rd());
double randf()
{
    return dis(gen);
}

// ��λ���ڵ��������
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

// ��������������
glm::vec3 randomDirection(glm::vec3 n)
{
    /*
    // �������
    glm::vec3 d;
    do
    {
        d = randomVec3();
    } while (glm::dot(d, n) < 0.0f);
    return d;
    */
    // ������
    return glm::normalize(randomVec3() + n);
}

float angleBetweenVectors(const glm::vec3& a, const glm::vec3& b) {
    // ������
    float dotProduct = glm::dot(a, b);

    // ������������
    float magnitudeA = glm::length(a);
    float magnitudeB = glm::length(b);

    // �����������������Ϊ��������
    if (magnitudeA < 1e-6 || magnitudeB < 1e-6) {
        return 0.0f; // ���׳��쳣������������
    }

    // ����нǵ�����ֵ
    float cosTheta = dotProduct / (magnitudeA * magnitudeB);

    // ʹ�÷����һ�ȡ����ֵ����ת��Ϊ�Ƕ�
    float angleRadians = glm::acos(cosTheta);
    float angleDegrees = glm::degrees(angleRadians);

    return angleDegrees; // ���ؽǶ�ֵ��0��~180�㣩
}

// ������ɫ���㣨������ײ���㣩
Vec3 ray_color(const Ray& ray, const Scene& scene, const Camera& cam) {
    HitRecord rec;
    float depth = 1.0f;
    //��������
    if (scene.hit(ray, 0.0f, FLT_MAX, rec)) {
        glm::vec3 lk = cam.lookat;
        float depth = glm::distance(rec.point ,cam.position)*abs(cos(angleBetweenVectors(lk,glm::vec3(rec.point-cam.position))));

       return Vec3(glm::clamp(depth, 0.0f, 1.0f));
        //return 0.5f * (rec.normal + glm::vec3(1.0f));
    }

    // ��������
    //glm::vec3 unit_dir = glm::normalize(ray.direction);
    //float t = 0.5f * (unit_dir.y + 1.0f);
    //return (1.0f - t) * Vec3(1.0f) + t * Vec3(0.5f, 0.7f, 1.0f);
    return Vec3(0);
}

Vec3 ray_color1(const Ray& ray, const Scene& scene, const Camera& cam) {
    HitRecord rec;
    // ʹ������Ľ�ƽ���Զƽ����Ϊ���߲�����Χ
    if (scene.hit(ray, cam.zNear, cam.zFar, rec)) {
        // �������߷��򣨼��� cam.lookat ����������Ŀ��㣩
        glm::vec3 viewDir = glm::normalize(cam.lookat - cam.position);
        // ���е㵽���������
        glm::vec3 toPoint = rec.point - cam.position;
        // �����߷����ͶӰ��ȣ�������ȣ�
        float depth = glm::dot(toPoint, viewDir);
        // ��������Բ�ֵ�� [0,1]
        float normalizedDepth = (depth - cam.zNear) / (cam.zFar - cam.zNear);
        normalizedDepth = glm::clamp(normalizedDepth, 0.0f, 1.0f);
        // ���ػҶ���ɫ��0Ϊ��ɫ��1Ϊ��ɫ��
        return Vec3(1-10*normalizedDepth);
    }

    return Vec3(0);
}

// �޸�ray_color����
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

        // ��Ӱ���
        Ray shadow_ray(rec.point + rec.normal * 0.001f, light_dir);
        if (scene.is_shadowed(shadow_ray, distance)) continue;

        // ���ռ���
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
    //�������
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

// ����������峡��
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

// ����Ⱦ����
void render_scene() {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    std::vector<PointLight> lights = {
        PointLight(glm::vec3(2,2,2)),
        PointLight(glm::vec3(2,2,-2))// ��Դλ��
    };

    //���
    Triangle l1 = Triangle(glm::vec3(0.6, 0.99, 0.4), glm::vec3(-0.2, 0.99, -0.4), glm::vec3(-0.2,
        0.99, 0.4), white_mat);
    l1.material->isLight = true;
    Triangle l2 = Triangle(glm::vec3(0.6, 0.99, 0.4), glm::vec3(0.6, 0.99, -0.4), glm::vec3(-0.2,
        0.99, -0.4), white_mat);
    l2.material->isLight = true;

    //ObjMesh obj = load_obj("square.obj");
    //Mesh mesh(obj);

    Model model("C:/Users/25342/OneDrive/����/Monte-Carlo-raytracer/MonteCarlo/obj/cube.obj");
    Mesh mesh(model,red_mat);
    
    // ��������
    Scene scene;
    Sphere sphere(glm::vec3(0, 0, -1), 0.5f,red_mat);
    Triangle tri(glm::vec3(-0.5, -0.5, -0.5), glm::vec3(0.5, -0.5, -0.5), glm::vec3(0, -0.5, 0.5),red_mat);
    Triangle tri1(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0),red_mat);
    Sphere ground(glm::vec3(0, -1000, 0), 999.0f, ground_mat);

    //auto objects = create_stress_scene(); // ����121������
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

    // �������
    Camera cam(
        glm::vec3(0, 0, 4),
        glm::vec3(0, 0, -1),
        90.0f,
        float(WIDTH) / HEIGHT
    );

    std::vector<uint8_t> pixels(WIDTH * HEIGHT * 3);

    omp_set_num_threads(50);
#pragma omp parallel for
    // ��������Ⱦ
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