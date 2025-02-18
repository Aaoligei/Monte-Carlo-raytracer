#pragma once
#include"glm.hpp"
#include<vector>
#include <iostream>
#include"Ray.h"
#include"obj_loader.h"
#include"Model.h"
#include"Material.h"
#include <algorithm>

// 命中记录结构
struct AABB {
    glm::vec3 min; // 最小坐标
    glm::vec3 max; // 最大坐标

    AABB() = default;
    AABB(const glm::vec3& min, const glm::vec3& max)
        : min(min), max(max) {}

    // 扩展包围盒以包含另一点
    void merge(const glm::vec3& point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    // 合并两个包围盒
    void merge(const AABB& other) {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);
    }

    static AABB surrounding_box(const AABB& box0, const AABB& box1) {
        glm::vec3 new_min(
            std::min(box0.min.x, box1.min.x),
            std::min(box0.min.y, box1.min.y),
            std::min(box0.min.z, box1.min.z)
        );
        glm::vec3 new_max(
            std::max(box0.max.x, box1.max.x),
            std::max(box0.max.y, box1.max.y),
            std::max(box0.max.z, box1.max.z)
        );
        return { new_min, new_max };
    }

    // 计算包围盒的大小
    glm::vec3 getSize() const {
        return max - min;
    }

    // 判断光线是否与包围盒相交
    bool hit(const Ray& ray, float t_min, float t_max) const {
        for (int a = 0; a < 3; a++) {
            float invD = 1.0f / ray.direction[a];
            float t0 = (min[a] - ray.origin[a]) * invD;
            float t1 = (max[a] - ray.origin[a]) * invD;
            if (invD < 0.0f)
                std::swap(t0, t1);
            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;
            if (t_max <= t_min)
                return false;
        }
        return true;
    }
};

struct HitRecord {
    float t;
    glm::vec3 point;
    glm::vec3 normal;
    bool front_face;
    glm::vec3 color; 
    std::shared_ptr<Material> material;

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

    virtual const Material* get_material() const = 0; // 改为返回材质指针

    virtual AABB bounding_box() const = 0;
};

class BVHNode : public Hittable {
    std::shared_ptr<Hittable> left;
    std::shared_ptr<Hittable> right;
    AABB box;

public:
    BVHNode(std::vector<std::shared_ptr<Hittable>>& objects,
        size_t start, size_t end)
    {
        int axis = rand() % 3; // 随机选择分割轴
        auto comparator = [axis](const auto& a, const auto& b) {
            return a->bounding_box().min[axis] < b->bounding_box().min[axis];
            };

        size_t span = end - start;
        if (span == 1) {
            left = right = objects[start];
        }
        else if (span == 2) {
            left = objects[start];
            right = objects[start + 1];
        }
        else {
            std::sort(objects.begin() + start, objects.begin() + end, comparator);
            size_t mid = start + span / 2;
            left = std::make_shared<BVHNode>(objects, start, mid);
            right = std::make_shared<BVHNode>(objects, mid, end);
        }

        box = AABB::surrounding_box(left->bounding_box(),
            right->bounding_box());
    }

    bool hit(const Ray& ray, float t_min, float t_max,
        HitRecord& rec) const override
    {
        if (!box.hit(ray, t_min, t_max)) return false;

        bool hit_left = left->hit(ray, t_min, t_max, rec);
        bool hit_right = right->hit(ray, t_min, hit_left ? rec.t : t_max, rec);
        return hit_left || hit_right;
    }

    AABB bounding_box() const override { return box; }
    const Material* get_material() const override {
        return NULL;
    }
};

// 球体类
class Sphere : public Hittable {
public:
    glm::vec3 center;
    float radius;
    std::shared_ptr<Material> material;

    Sphere(const glm::vec3& c, float r, std::shared_ptr<Material> material) : center(c), radius(r),material(material) {}

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
        rec.material = material;
        return true;
    }

    const Material* get_material() const override {
        return material.get();
    }

    AABB bounding_box() const override {
        glm::vec3 radius_vec(radius);
        return { center - radius_vec, center + radius_vec };
    }
};

// 场景类
class Scene : public Hittable {
public:
    std::shared_ptr<Hittable> objects;
    std::shared_ptr<Material> material;
    AABB aabb;

   
    void build_bvh(std::vector<std::shared_ptr<Hittable>> object) {
        objects = std::make_shared<BVHNode>(object, 0, object.size());
    }

    bool hit(const Ray& ray, float t_min, float t_max,
        HitRecord& rec) const override
    {
        return objects->hit(ray, t_min, t_max, rec);
    }
    

    bool is_shadowed(const Ray& shadow_ray, float max_dist) const {
        HitRecord temp_rec;
        return hit(shadow_ray, 0.001f, max_dist, temp_rec);
    }

    const Material* get_material() const override {
        return material.get();
    }

    virtual AABB bounding_box() const override {
        return aabb;
    }
};

//三角类 Möller-Trumbore算法
class Triangle : public Hittable {
public:
    glm::vec3 v0, v1, v2;
    glm::vec3 normal;
    std::shared_ptr<Material> material;

    Triangle(const glm::vec3& a,
        const glm::vec3& b,
        const glm::vec3& c,
        std::shared_ptr<Material> material)
        : v0(a), v1(b), v2(c) ,material(material){
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
        rec.material = material;
        return true;
    }

    const Material* get_material() const override {
        return material.get();
    }

    AABB bounding_box() const override {
        return {
            glm::min(v0, glm::min(v1, v2)),
            glm::max(v0, glm::max(v1, v2))
        };
    }
};

// mesh.h
class Mesh : public Hittable {
public:
    std::vector<Triangle> triangles;
    std::shared_ptr<Material> material;
    AABB aabb;

    Mesh(const Model& obj, std::shared_ptr<Material> material)
        : material(material) {
        // 初始化 AABB
        aabb = AABB(glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX), glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX));

        // 生成三角形
        for (size_t i = 0; i < obj.faces.size(); i++) {
            glm::vec3 v0 = glm::vec3(obj.vertexes[obj.faces[i].vertexIdx[0]].point.x,
                obj.vertexes[obj.faces[i].vertexIdx[0]].point.y, obj.vertexes[obj.faces[i].vertexIdx[0]].point.z);
            glm::vec3 v1 = glm::vec3(obj.vertexes[obj.faces[i].vertexIdx[1]].point.x,
                obj.vertexes[obj.faces[i].vertexIdx[1]].point.y, obj.vertexes[obj.faces[i].vertexIdx[1]].point.z);
            glm::vec3 v2 = glm::vec3(obj.vertexes[obj.faces[i].vertexIdx[2]].point.x,
                obj.vertexes[obj.faces[i].vertexIdx[2]].point.y, obj.vertexes[obj.faces[i].vertexIdx[2]].point.z);

            Triangle tri(v0, v1, v2, material);
            tri.normal = glm::vec3(obj.faces[i].normal.x, obj.faces[i].normal.y, obj.faces[i].normal.z);

            // 更新 AABB
            aabb.merge(v0);
            aabb.merge(v1);
            aabb.merge(v2);

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

    const Material* get_material() const override {
        return material.get();
    }

    virtual AABB bounding_box() const override {
        return aabb;
    }
};

