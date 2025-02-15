#pragma once
// obj_loader.h
#include <vector>
#include <fstream>
#include <sstream>
#include <glm.hpp>

struct ObjMesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<unsigned> indices;
};

ObjMesh load_obj(const std::string& path) {
    ObjMesh mesh;
    std::ifstream file(path);
    std::string line;

    while (getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {  // 顶点
            glm::vec3 v;
            iss >> v.x >> v.y >> v.z;
            mesh.vertices.push_back(v);
        }
        else if (type == "vn") {  // 法线
            glm::vec3 n;
            iss >> n.x >> n.y >> n.z;
            mesh.normals.push_back(n);
        }
        else if (type == "f") {  // 面
            unsigned v[3], t[3], n[3];
            char slash;
            for (int i = 0; i < 3; ++i) {
                iss >> v[i] >> slash >> t[i] >> slash >> n[i];
                mesh.indices.push_back(v[i] - 1);  // OBJ索引从1开始
            }
        }
    }
    return mesh;
}