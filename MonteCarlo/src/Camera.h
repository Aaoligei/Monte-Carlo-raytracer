#pragma once
#include<glm.hpp>
#include "Ray.h"
// 相机类
class Camera {
public:
    glm::vec3 position;
    glm::vec3 lookat;
    float vfov;//垂直视场角
    float aspect_ratio;
    float zNear;
    float zFar;

    Camera(const glm::vec3& pos,
        const glm::vec3& target,
        float vertical_fov,
        float ratio = 16.0f / 9.0f,
        float zNear=0.1f,
        float zFar=100.0f)
        : position(pos),
        lookat(target),
        vfov(vertical_fov),
        aspect_ratio(ratio),
        zNear(zNear),
        zFar(zFar){}

    //s和t分别对应u和v;s=0, t=0 → 光线指向视口左下角 ;s = 1, t = 1 → 光线指向视口右上角; 中间值线性插值
    Ray generateRay(float s, float t) const {
        float theta = glm::radians(vfov);//度数转为弧度
        float viewport_height = 2.0f * tan(theta / 2.0f);
        float viewport_width = aspect_ratio * viewport_height;

        glm::vec3 w = glm::normalize(position - lookat); //相机前方向(指向观察点反方向)
        glm::vec3 u = glm::normalize(glm::cross(glm::vec3(0, 1, 0), w));//相机右方向 (世界坐标系Y轴与w的叉积)
        glm::vec3 v = glm::cross(w, u);//相机上方向 (w与u的叉积)

        glm::vec3 horizontal = viewport_width * u;
        glm::vec3 vertical = viewport_height * v;
        glm::vec3 lower_left = position - horizontal / 2.0f - vertical / 2.0f - w;

        return Ray(position,
            lower_left + s * horizontal + t * vertical - position);
    }
};