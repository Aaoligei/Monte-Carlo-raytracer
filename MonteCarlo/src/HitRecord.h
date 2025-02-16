#pragma once
#include"glm.hpp"
#include<vector>
#include <iostream>
#include"Ray.h"
#include"obj_loader.h"
#include"Model.h"
// 命中记录结构
struct HitRecord {
    float t;
    glm::vec3 point;
    glm::vec3 normal;
    bool front_face;
    glm::vec3 color; 

    void set_face_normal(const Ray& ray, const glm::vec3& outward_normal) {
        front_face = glm::dot(ray.direction, outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// 可命中对象基类
class Hittable {
public:
    virtual ~Hittable() = default;
    virtual glm::vec3 get_color() const { return glm::vec3(1.0f); }
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

//三角类 Möller-Trumbore算法
class Triangle : public Hittable {
public:
    glm::vec3 v0, v1, v2;
    glm::vec3 normal;

    Triangle(const glm::vec3& a,
        const glm::vec3& b,
        const glm::vec3& c)
        : v0(a), v1(b), v2(c) {
        normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
    }

    bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const override {
        const float EPSILON = 1e-6f;
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 h = glm::cross(ray.direction, edge2);
        float a = glm::dot(edge1, h);

        if (a > -EPSILON && a < EPSILON)
            return false; // 光线平行于三角形

        float f = 1.0f / a;
        glm::vec3 s = ray.origin - v0;
        float u = f * glm::dot(s, h);
        if (u < 0.0f || u > 1.0f)
            return false;

        glm::vec3 q = glm::cross(s, edge1);
        float v = f * glm::dot(ray.direction, q);
        if (v < 0.0f || u + v > 1.0f)
            return false;

        float t = f * glm::dot(edge2, q);
        if (t < t_min || t > t_max)
            return false;

        rec.t = t;
        rec.point = ray.at(t);
        rec.normal = normal;
        return true;
    }
};

// mesh.h
class Mesh : public Hittable {
public:
    std::vector<Triangle> triangles;


    Mesh(const Model& obj) {
        
        // 生成三角形
        for (size_t i = 0; i < obj.faces.size(); i ++) {
            glm::vec3 v0 = glm::vec3(obj.vertexes[obj.faces[i].vertexIdx[0]].point.x, obj.vertexes[obj.faces[i].vertexIdx[0]].point.y, obj.vertexes[obj.faces[i].vertexIdx[0]].point.z);
            glm::vec3 v1 = glm::vec3(obj.vertexes[obj.faces[i].vertexIdx[1]].point.x, obj.vertexes[obj.faces[i].vertexIdx[1]].point.y, obj.vertexes[obj.faces[i].vertexIdx[1]].point.z);
            glm::vec3 v2 = glm::vec3(obj.vertexes[obj.faces[i].vertexIdx[2]].point.x, obj.vertexes[obj.faces[i].vertexIdx[2]].point.y, obj.vertexes[obj.faces[i].vertexIdx[2]].point.z);
            Triangle tri(v0, v1, v2);
            triangles.emplace_back(tri);
        }
        std::cout << "mesh面数：" << triangles.size() << std::endl;
    }

    bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const override {
        bool hit_anything = false;
        HitRecord temp_rec;

        for (const auto& tri : triangles) {
            if (tri.hit(ray, t_min, t_max, temp_rec)) {
                hit_anything = true;
                t_max = temp_rec.t;
                rec = temp_rec;
            }
        }
        return hit_anything;
    }
};

// cylinder.h
class Cylinder : public Hittable {
    glm::vec3 start;
    glm::vec3 end;
    float radius;
    glm::vec3 color;
    glm::vec3 axis;

public:
    Cylinder(const glm::vec3& s, const glm::vec3& e,
        float r, const glm::vec3& col)
        : start(s), end(e), radius(r), color(col) {
        axis = glm::normalize(end - start);
    }

    glm::vec3 get_color() const override { return color; }

    bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const override {
        const float EPSILON = 1e-6f;
        glm::vec3 dir = end - start;
        float length = glm::length(dir);
        glm::vec3 unit_dir = dir / length;

        // 将光线转换到圆柱局部坐标系
        glm::vec3 oc = ray.origin - start;
        float proj = glm::dot(oc, unit_dir);
        glm::vec3 oc_proj = proj * unit_dir;
        glm::vec3 oc_perp = oc - oc_proj;

        glm::vec3 ray_dir_perp = ray.direction - glm::dot(ray.direction, unit_dir) * unit_dir;

        // 计算圆柱相交
        float a = glm::dot(ray_dir_perp, ray_dir_perp);
        float b = 2.0f * glm::dot(oc_perp, ray_dir_perp);
        float c = glm::dot(oc_perp, oc_perp) - radius * radius;

        float discriminant = b * b - 4 * a * c;
        if (discriminant < 0) return false;

        float sqrtd = sqrt(discriminant);
        float root = (-b - sqrtd) / (2 * a);
        if (root < t_min || root > t_max) {
            root = (-b + sqrtd) / (2 * a);
            if (root < t_min || root > t_max)
                return false;
        }

        // 检查高度范围
        glm::vec3 hit_point = ray.at(root);
        glm::vec3 vec_to_start = hit_point - start;
        float projection = glm::dot(vec_to_start, unit_dir);
        if (projection < 0 || projection > length) return false;

        // 记录命中信息
        rec.t = root;
        rec.point = hit_point;
        rec.normal = glm::normalize((hit_point - start) - projection * unit_dir);
        rec.color = color;
        return true;
    }
};