#include <iostream>
#include <vector>
#include <glm.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>


// 基础向量类
class Vec3 : public glm::vec3 {
public:
    using glm::vec3::vec3;
    Vec3(const glm::vec3& v) : glm::vec3(v) {}

    uint8_t r() const { return static_cast<uint8_t>(255 * glm::clamp(x, 0.0f, 1.0f)); }
    uint8_t g() const { return static_cast<uint8_t>(255 * glm::clamp(y, 0.0f, 1.0f)); }
    uint8_t b() const { return static_cast<uint8_t>(255 * glm::clamp(z, 0.0f, 1.0f)); }
};

// 光线类
class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(const glm::vec3& o, const glm::vec3& d)
        : origin(o), direction(glm::normalize(d)) {}

    glm::vec3 at(float t) const {
        return origin + t * direction;
    }
};

// 命中记录结构
struct HitRecord {
    float t;
    glm::vec3 point;
    glm::vec3 normal;
    bool front_face;

    void set_face_normal(const Ray& ray, const glm::vec3& outward_normal) {
        front_face = glm::dot(ray.direction, outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// 可命中对象基类
class Hittable {
public:
    virtual ~Hittable() = default;
    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const = 0;
};

// 球体类
class Sphere : public Hittable {
public:
    glm::vec3 center;
    float radius;

    Sphere(const glm::vec3& c, float r) : center(c), radius(r) {}

    bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const override {
        glm::vec3 oc = ray.origin - center;
        float a = glm::dot(ray.direction, ray.direction);
        float b = 2.0f * glm::dot(oc, ray.direction);
        float c = glm::dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;

        if (discriminant < 0) return false;

        float sqrtd = sqrt(discriminant);
        float root = (-b - sqrtd) / (2.0f * a);
        if (root < t_min || root > t_max) {
            root = (-b + sqrtd) / (2.0f * a);
            if (root < t_min || root > t_max)
                return false;
        }

        rec.t = root;
        rec.point = ray.at(rec.t);
        glm::vec3 outward_normal = (rec.point - center) / radius;
        rec.set_face_normal(ray, outward_normal);
        return true;
    }
};

// 场景类
class Scene : public Hittable {
public:
    std::vector<std::shared_ptr<Hittable>> objects;

    void add(std::shared_ptr<Hittable> object) {
        objects.push_back(object);
    }

    bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const override {
        HitRecord temp_rec;
        bool hit_anything = false;
        auto closest_so_far = t_max;

        for (const auto& object : objects) {
            if (object->hit(ray, t_min, closest_so_far, temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }
};

// 相机类
class Camera {
public:
    glm::vec3 position;
    glm::vec3 lookat;
    float vfov;
    float aspect_ratio;

    Camera(const glm::vec3& pos,
        const glm::vec3& target,
        float vertical_fov,
        float ratio = 16.0f / 9.0f)
        : position(pos),
        lookat(target),
        vfov(vertical_fov),
        aspect_ratio(ratio) {}

    Ray generateRay(float s, float t) const {
        float theta = glm::radians(vfov);
        float viewport_height = 2.0f * tan(theta / 2.0f);
        float viewport_width = aspect_ratio * viewport_height;

        glm::vec3 w = glm::normalize(position - lookat);
        glm::vec3 u = glm::normalize(glm::cross(glm::vec3(0, 1, 0), w));
        glm::vec3 v = glm::cross(w, u);

        glm::vec3 horizontal = viewport_width * u;
        glm::vec3 vertical = viewport_height * v;
        glm::vec3 lower_left = position - horizontal / 2.0f - vertical / 2.0f - w;

        return Ray(position,
            lower_left + s * horizontal + t * vertical - position);
    }
};

// 光线颜色计算
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

// 主渲染函数
void render_scene() {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    // 场景设置
    Scene scene;
    scene.add(std::make_shared<Sphere>(glm::vec3(0, 0, -1), 0.5f));
    scene.add(std::make_shared<Sphere>(glm::vec3(0, -100.5, -1), 100.0f));

    // 相机配置
    Camera cam(
        glm::vec3(0, 0, 1),
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