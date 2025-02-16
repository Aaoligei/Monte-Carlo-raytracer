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

#include <gtc/constants.hpp> // �����г���

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

        return 0.5f * (rec.normal + glm::vec3(1.0f));
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
        return Vec3(10*normalizedDepth);
    }

    return Vec3(0);
}

// ����Ⱦ����
void render_scene() {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    //ObjMesh obj = load_obj("square.obj");
    //Mesh mesh(obj);

    Model model("C:/Users/25342/OneDrive/����/Monte-Carlo-raytracer/MonteCarlo/src/square.obj");
    Mesh mesh(model);
    
    // ��������
    Scene scene;
    Sphere sphere(glm::vec3(0, 0, -1), 0.5f);
    Triangle tri(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0));

    //scene.add(std::make_shared<Sphere>(sphere));
    //scene.add(std::make_shared<Triangle>(tri));
    //scene.add(std::make_shared<Sphere>(glm::vec3(0, -100.5, -1), 100.0f));
    scene.add(std::make_shared<Mesh>(mesh));

    // �������
    Camera cam(
        glm::vec3(1, 1, 3),
        glm::vec3(0, 0, -1),
        90.0f,
        float(WIDTH) / HEIGHT
    );

    std::vector<uint8_t> pixels(WIDTH * HEIGHT * 3);

    // ��������Ⱦ
    for (int j = 0; j < HEIGHT; ++j) {
        for (int i = 0; i < WIDTH; ++i) {
            float u = float(i) / (WIDTH - 1);
            float v = float(HEIGHT - 1 - j) / (HEIGHT - 1);

            Ray ray = cam.generateRay(u, v);
            Vec3 color = ray_color1(ray, scene,cam);

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