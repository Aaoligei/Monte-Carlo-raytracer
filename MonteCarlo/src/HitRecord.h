#pragma once
#include"glm.hpp"
#include<vector>
#include <iostream>
#include"Ray.h"
#include"obj_loader.h"
#include"Model.h"
#include"Material.h"
#include <algorithm>



struct HitRecord {
    float t;
    glm::vec3 point;
    glm::vec3 normal;
    bool front_face;
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


};
//三角类 Möller-Trumbore算法
class Triangle : public Hittable {
public:
    glm::vec3 v0, v1, v2, center;
    glm::vec3 normal;
    std::shared_ptr<Material> material=RED;

    Triangle(const glm::vec3& a,
        const glm::vec3& b,
        const glm::vec3& c,
        std::shared_ptr<Material> material)
        : v0(a), v1(b), v2(c), material(material) {
        normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        center = (v1 + v2 + v0) / glm::vec3(3, 3, 3);
    }

    Triangle(const Triangle& other) :
        v0(other.v0), v1(other.v1), v2(other.v2),
        center(other.center), normal(other.normal),
        material(other.material) {}



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

    // 按照三角形中心排序 -- 比较函数
    static bool cmpx(const Triangle& t1, const Triangle& t2) {
        return t1.center.x < t2.center.x;
    }
    static bool cmpy(const Triangle& t1, const Triangle& t2) {
        return t1.center.y < t2.center.y;
    }
    static bool cmpz(const Triangle& t1, const Triangle& t2) {
        return t1.center.z < t2.center.z;
    }

};

// 场景类
class Scene : public Hittable {
public:
    std::vector<std::shared_ptr<Hittable>> objects;
    std::shared_ptr<Material> material;
   
    void add(std::shared_ptr<Hittable> object) {
        objects.push_back(object);
    }

    bool hit(const Ray& ray, float t_min, float t_max,
        HitRecord& rec) const override 
    {
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

    bool is_shadowed(const Ray& shadow_ray, float max_dist) const {
        HitRecord temp_rec;
        return hit(shadow_ray, 0.001f, max_dist, temp_rec);
    }

    

};


// mesh.h
class Mesh : public Hittable {
public:
    std::vector<Triangle> triangles;
    std::shared_ptr<Material> material;

    Mesh(const Model& obj, std::shared_ptr<Material> material)
        : material(material) {

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
struct BVHNode {
    BVHNode* left = nullptr;
    BVHNode* right = nullptr;
    int n = 0, index = 0;
    glm::vec3 AA, BB;
};

// BVH树
class BVHTree:public Hittable {
public:
    BVHNode* root = nullptr;
    std::vector<Triangle> triangles;
    ProcessBar bvhbar;

    bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const override {
        HitRecord temp_rec;
        bool hit_anything = false;
        auto closest_so_far = t_max;

        if (root == nullptr) return false;
        std::vector<BVHNode*> stack;
        stack.push_back(root);
        while (!stack.empty()) {
            BVHNode* node = stack.back();
            stack.pop_back();
            if (node == nullptr) continue;

            // 使用Ray类中的hitAABB方法检查光线与AABB是否相交
            if (!ray.hitAABB(node->AA, node->BB, t_min, closest_so_far)) continue;

            if (node->n >= 1) {
                for (int i = node->index; i < node->index + node->n; i++) {
					if (i >= triangles.size()) continue;
                    if (triangles[i].hit(ray, t_min, closest_so_far, temp_rec)) {
                        hit_anything = true;
                        closest_so_far = temp_rec.t;
                        rec = temp_rec;
                    }
                }
            }
            else {
                // 先把右子节点加入栈中，这样左子节点会先被处理
                if (node->right != nullptr) stack.push_back(node->right);
                if (node->left != nullptr) stack.push_back(node->left);
            }
        }

        return hit_anything;
    }

    BVHTree(std::vector<Triangle>& triangles, int l, int r, int n) {
        this->triangles.clear();
        // 复制三角形，确保材质指针也被复制
        for (auto& tri : triangles) {
            this->triangles.push_back(tri);
        }
        bvhbar.total = this->triangles.size();
        bvhbar.processed = 0;
        root=buildBVH(this->triangles, l, r, n);
		
    }
   
private:
    BVHNode* buildBVH(std::vector<Triangle>& triangles, int l, int r, int n) {
		
        if (l > r) return 0;
        BVHNode* node = new BVHNode();
        node->AA = glm::vec3(INF, INF, INF);
        node->BB = glm::vec3(-INF, -INF, -INF);

        // 计算 AABB
        for (int i = l; i <= r; i++) {
            // 最小点 AA
            float minx = std::min(triangles[i].v0.x, std::min(triangles[i].v1.x,
                triangles[i].v2.x));
            float miny = std::min(triangles[i].v0.y, std::min(triangles[i].v1.y,
                triangles[i].v2.y));
            float minz = std::min(triangles[i].v0.z, std::min(triangles[i].v1.z,
                triangles[i].v2.z));
            node->AA.x = std::min(node->AA.x, minx);
            node->AA.y = std::min(node->AA.y, miny);
            node->AA.z = std::min(node->AA.z, minz);
            // 最大点 BB
            float maxx = std::max(triangles[i].v0.x, std::max(triangles[i].v1.x,
                triangles[i].v2.x));
            float maxy = std::max(triangles[i].v0.y, std::max(triangles[i].v1.y,
                triangles[i].v2.y));
            float maxz = std::max(triangles[i].v0.z, std::max(triangles[i].v1.z,
                triangles[i].v2.z));
            node->BB.x = std::max(node->BB.x, maxx);
            node->BB.y = std::max(node->BB.y, maxy);
            node->BB.z = std::max(node->BB.z, maxz);
        }
        // 不多于 n 个三角形 返回叶子节点
        if ((r - l + 1) <= n) {
            node->n = r - l + 1;
            node->index = l;
			bvhbar.update(bvhbar.processed + node->n);
            printProgress(bvhbar);
            return node;
        }
        // 否则递归建树
        float lenx = node->BB.x - node->AA.x;
        float leny = node->BB.y - node->AA.y;
        float lenz = node->BB.z - node->AA.z;

        // 按 x 划分
        if (lenx >= leny && lenx >= lenz)
            std::sort(triangles.begin() + l, triangles.begin() + r + 1, Triangle::cmpx);
        // 按 y 划分
        if (leny >= lenx && leny >= lenz)
            std::sort(triangles.begin() + l, triangles.begin() + r + 1, Triangle::cmpy);
        // 按 z 划分
        if (lenz >= lenx && lenz >= leny)
            std::sort(triangles.begin() + l, triangles.begin() + r + 1, Triangle::cmpz);
        // 递归
        int mid = (l + r) / 2;
        node->left = buildBVH(triangles, l, mid, n);
        node->right = buildBVH(triangles, mid + 1, r, n);
        return node;
    }
};
