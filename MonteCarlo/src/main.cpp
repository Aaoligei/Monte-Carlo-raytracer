#include <iostream>
#include <vector>
#include <glm.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>
#include <gtc/matrix_transform.hpp>
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

#include <gtc/constants.hpp> // �����г���
#include <chrono>
const int SAMPLE = 1024;
const double LIGHT_INTENSITY = 1.2;
const double BRIGHTNESS = (2.0f * 3.1415926f) * (1.0f / double(SAMPLE)) * LIGHT_INTENSITY;
const int WIDTH = 512;
const int HEIGHT = 512;

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

Vec3 PathTracing(const Ray& ray, const Scene& scene ,int depth) {
    if (depth > 8) return glm::vec3(0);

    HitRecord rec;
    if (!scene.hit(ray, 0.001f, FLT_MAX, rec)) {
        return Vec3(0);
    }
    if (rec.material->isLight) {
        return rec.material->color;
    }
    else {
        // �� P �ĸ�����ֹ
        double r = randf();
        float P = 0.8;
        if (r > P) return glm::vec3(0);

        Ray randomRay(rec.point, randomDirection(rec.normal));

        //�������
        float cosine = glm::dot(-ray.direction, rec.normal);
        Vec3 color = Vec3(0);

        // ���ݷ����ʾ����������յķ���
        r = randf();
        if (r < rec.material->specularRate)  // ���淴��
        {
            randomRay.direction = normalize(glm::reflect(ray.direction,
                rec.normal));
            color = PathTracing(randomRay, scene,  depth + 1) * cosine;
        }
        else {
            Vec3 srcColor = rec.material->color;
            Vec3 ptColor = PathTracing(randomRay, scene, depth + 1) * cosine;
            color = ptColor * srcColor;
        }

        return color/P;
    }
}

Vec3 CalColor(const Ray& ray, const Scene& scene) {
    HitRecord rec;
    if (!scene.hit(ray, 0.001f, FLT_MAX, rec)) {
        return Vec3(0);
    }
    if (rec.material->isLight) {
        return rec.material->color;
    }
    Ray randomRay(rec.point, randomDirection(rec.normal));
    Vec3 color = Vec3(0);
    // ���ݷ����ʾ����������յķ���
    double r = randf();
    if (r < rec.material->specularRate)  // ���淴��
    {
        randomRay.direction = normalize(glm::reflect(ray.direction,
            rec.normal));
        color = PathTracing(randomRay, scene, 0);
    }
    else {
        Vec3 srcColor = rec.material->color;
        Vec3 ptColor = PathTracing(randomRay, scene, 0);
        color = ptColor * srcColor;
    }

    return Vec3(color.x * BRIGHTNESS, color.y * BRIGHTNESS, color.z * BRIGHTNESS);
}

// ����Ⱦ����
void render_scene() {
    // ��������
    Scene scene;

    //���
    Triangle l1 = Triangle( glm::vec3(-0.7, 0.99, -0.7), glm::vec3(0.7, 0.99, 0.7), glm::vec3(-0.7,
        0.99, 0.7), WHITE);
    l1.material->isLight = true;
    Triangle l2 = Triangle(glm::vec3(0.7, 0.99, 0.7), glm::vec3(0.7, 0.99, -0.7), glm::vec3(-0.7,
        0.99, -0.7), WHITE);
    l2.material->isLight = true;
    /*Triangle l3 = Triangle(glm::vec3(0.99, 0.7, 0.7), glm::vec3(0.99, -0.7, -0.7), glm::vec3(0.99, -0.7, 0.7), WHITE);
    l1.material->isLight = true;
    Triangle l4 = Triangle(glm::vec3(0.99, -0.7, -0.7), glm::vec3(0.99, 0.7, 0.7), glm::vec3(0.99, 0.7, -0.7), WHITE);
    l2.material->isLight = true;*/

	//��ȡģ��
    Model model("C:/Users/25342/OneDrive/����/Monte-Carlo-raytracer/MonteCarlo/obj/monkey.obj");
    model.scale(0.5);

	//��������(������)
    Mesh mesh(model,YELLOW);
    //����bvh
    BVHTree bvhtree(mesh.triangles, 0, mesh.triangles.size(), 5);
    scene.add(std::make_shared<BVHTree>(bvhtree));
    
	// ��������
    Sphere sphere(glm::vec3(-0.6, -0.8, 0.2), 0.2f, GREEN_MIRROR);
    scene.add(std::make_shared<Sphere>(sphere));

	//������
    scene.add(std::make_shared<Triangle>(l1));
    scene.add(std::make_shared<Triangle>(l2));
    //scene.add(std::make_shared<Triangle>(l3));
    //scene.add(std::make_shared<Triangle>(l4));

	// ������
    //scene.add(std::make_shared<Triangle>(glm::vec3(-0.15, 0.4, -0.6), glm::vec3(-0.15, -0.95, -0.6), glm::vec3(0.15, 0.4, -0.6), YELLOW));
    //scene.add(std::make_shared<Triangle>(glm::vec3(0.15, 0.4, -0.6), glm::vec3(-0.15, -0.95, -0.6), glm::vec3(0.15, -0.95, -0.6), YELLOW));
    // ��������
    // bottom
    scene.add(std::make_shared<Triangle>(glm::vec3(1, -1, 1), glm::vec3(-1, -1, -1), glm::vec3(-1, -1, 1), GRAY));
    scene.add(std::make_shared<Triangle>( glm::vec3(1, -1, 1), glm::vec3(1, -1, -1), glm::vec3(-1, -1, -1), GRAY));
    // top
    scene.add(std::make_shared<Triangle>(glm::vec3(1, 1, 1), glm::vec3(-1, 1, 1), glm::vec3(-1, 1, -1), GRAY));
    scene.add(std::make_shared<Triangle>(glm::vec3(1, 1, 1), glm::vec3(-1, 1, -1), glm::vec3(1, 1, -1), GRAY));
    // back
    scene.add(std::make_shared<Triangle>(glm::vec3(1, -1, -1), glm::vec3(-1, 1, -1), glm::vec3(-1, -1, -1), GRAY));
    scene.add(std::make_shared<Triangle>(glm::vec3(1, -1, -1), glm::vec3(1, 1, -1), glm::vec3(-1, 1, -1), GRAY));
    // left
    scene.add(std::make_shared<Triangle>(glm::vec3(-1, -1, -1), glm::vec3(-1, 1, 1), glm::vec3(-1, -1, 1), GREEN));
    scene.add(std::make_shared<Triangle>(glm::vec3(-1, -1, -1), glm::vec3(-1, 1, -1), glm::vec3(-1, 1, 1), GREEN));
    // right
    scene.add(std::make_shared<Triangle>(glm::vec3(1, 1, 1), glm::vec3(1, -1, -1), glm::vec3(1, -1, 1), RED));
    scene.add(std::make_shared<Triangle>(glm::vec3(1, -1, -1), glm::vec3(1, 1, 1), glm::vec3(1, 1, -1), RED));

    // �������
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
    // ��������Ⱦ
   for (int k = 0; k < SAMPLE; k++) {
        for (int j = 0; j < HEIGHT; ++j) {
            for (int i = 0; i < WIDTH; ++i) {
                
                float u = float(i) / (WIDTH - 1);
                float v = float(HEIGHT - 1 - j) / (HEIGHT - 1);

                Ray ray = cam.shootRay(u, v);
                Vec3 color = CalColor(ray, scene);

                size_t index = 3 * (j * WIDTH + i);

                image[index] += color.r();
                image[index + 1] += color.g();
                image[index + 2] += color.b();

                
            }
        }
   }

   for (int i = 0; i < WIDTH * HEIGHT * 3; ++i) {
       // �� double ֵ���ŵ� 0-255 ��Χ�ڣ���ת��Ϊ uint8_t
       pixels[i] = static_cast<uint8_t>(std::min(255.0, std::max(0.0, image[i])));
   }

   // ���� image ����
   delete[] image;

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