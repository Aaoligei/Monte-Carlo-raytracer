#pragma once
#include<glm.hpp>
#include "Ray.h"
// �����
class Camera {
public:
    glm::vec3 position;
    glm::vec3 lookat;
    float vfov;//��ֱ�ӳ���
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

    //s��t�ֱ��Ӧu��v;s=0, t=0 �� ����ָ���ӿ����½� ;s = 1, t = 1 �� ����ָ���ӿ����Ͻ�; �м�ֵ���Բ�ֵ
    Ray generateRay(float s, float t) const {
        float theta = glm::radians(vfov);//����תΪ����
        float viewport_height = 2.0f * tan(theta / 2.0f);
        float viewport_width = aspect_ratio * viewport_height;

        glm::vec3 w = glm::normalize(position - lookat); //���ǰ����(ָ��۲�㷴����)
        glm::vec3 u = glm::normalize(glm::cross(glm::vec3(0, 1, 0), w));//����ҷ��� (��������ϵY����w�Ĳ��)
        glm::vec3 v = glm::cross(w, u);//����Ϸ��� (w��u�Ĳ��)

        glm::vec3 horizontal = viewport_width * u;
        glm::vec3 vertical = viewport_height * v;
        glm::vec3 lower_left = position - horizontal / 2.0f - vertical / 2.0f - w;

        return Ray(position,
            lower_left + s * horizontal + t * vertical - position);
    }
};