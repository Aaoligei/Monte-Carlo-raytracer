#include "Model.h"
#include <fstream>
#include <iostream>
#include <string>
#include <ctime>
#include <ext/matrix_transform.hpp>

using namespace std;

bool Model::load(const string& path)
{
	clock_t t = clock();
	ifstream file(path);
	if (!file.is_open()) return false;
	string type;

	while (file >> type)
	{
		if (type == "v")
		{
			Vertex vt;
			file >> vt.point.x >> vt.point.y >> vt.point.z;
			vertexes.push_back(vt);
		}

		else if (type == "f")
		{
			Face face;
			int vIndex, tIndex, nIndex;
			int faceIndex = faces.size();

			while (true)
			{
				char ch = file.get();
				if (ch == ' ') continue;
				else if (ch == '\n' || ch == EOF) break;
				else file.putback(ch);

				file >> vIndex;

				char splitter = file.get();
				nIndex = 0;

				if (splitter == '/')
				{
					splitter = file.get();
					if (splitter == '/')
					{
						file >> nIndex;
					}
					else
					{
						file.putback(splitter);
						file >> tIndex;
						splitter = file.get();
						if (splitter == '/')
						{
							file >> nIndex;
						}
						else file.putback(splitter);
					}
				}
				else file.putback(splitter);

				face.vertexIdx.push_back(vIndex - 1);
				face.normalIdx.push_back(nIndex - 1);
			}

			if (face.vertexIdx.size() > 2)
			{
				Point3f& a = vertexes[face.vertexIdx[0]].point,
					& b = vertexes[face.vertexIdx[1]].point, & c = vertexes[face.vertexIdx[2]].point;
				Vec3f normal = normalize(cross(b - a, c - b));

				face.normal = normal;
				faces.push_back(face);
			}
		}

		else if (type == "vn")
		{
			Vec3f vn;
			file >> vn.x >> vn.y >> vn.z;
			normals.push_back(vn);
		}
	}
	file.close();
	cout << "模型加载耗时：" << float(clock() - t) << "ms。" << endl;
	return true;
}

Model::Model(const string& path)
{

	if (load(path))
	{
		string face_type;
		cout << "模型" + path + "加载成功！" << endl;
		cout << "面元数：" << faces.size() <<
			"，顶点数：" << vertexes.size() << endl;
	}
	else
	{
		cout << "无法打开obj文件：" + path << endl;
	}
}


void Model::scale(float scaleFactor) {
	if (scaleFactor == 0.0f) {
		cout << "缩放因子不能为零。" << endl;
		return;
	}

	// 缩放顶点坐标（以模型中心为基准）
	int vertex_num = vertexes.size();
#pragma omp parallel for
	for (int i = 0; i < vertex_num; ++i) {
		Point3f translated = vertexes[i].point - center_point;
		translated.x *= scaleFactor;
		translated.y *= scaleFactor;
		translated.z *= scaleFactor;
		vertexes[i].point = translated + center_point;
	}

	// 缩放顶点法线并归一化
	int normal_num = normals.size();
#pragma omp parallel for
	for (int i = 0; i < normal_num; ++i) {
		normals[i].x /= scaleFactor;
		normals[i].y /= scaleFactor;
		normals[i].z /= scaleFactor;
		normals[i] = normalize(normals[i]);
	}

	// 缩放面法线并归一化
	int face_num = faces.size();
#pragma omp parallel for
	for (int i = 0; i < face_num; ++i) {
		faces[i].normal.x /= scaleFactor;
		faces[i].normal.y /= scaleFactor;
		faces[i].normal.z /= scaleFactor;
		faces[i].normal = normalize(faces[i].normal);
	}
}

void Model::rotate(float angle, const glm::vec3& axis) {
	if (vertexes.empty()) return;

	// 生成旋转矩阵
	glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f),glm::radians(angle), axis);

	// 旋转顶点
	for (auto& vertex : vertexes) {
		// 平移至原点并旋转
		glm::vec3 translated = glm::vec3(vertex.point.x, vertex.point.y, vertex.point.z) - glm::vec3(center_point.x, center_point.y, center_point.z);
		glm::vec3 rotated = rotationMat * glm::vec4(translated, 1.0f);
		glm::vec3 temp = glm::vec3(center_point.x, center_point.y, center_point.z);
		vertex.point = Vec3f(rotated.x + temp.x, rotated.y + temp.y, rotated.z + temp.z);
	}

	// 旋转顶点法线（若存在）
	for (auto& normal : normals) {
		glm::vec3 rotatedNormal = rotationMat * glm::vec4(normal.x,normal.y, normal.z, 0.0f);
		glm::vec3 temp = glm::normalize(rotatedNormal);
		normal = Vec3f(temp.x, temp.y, temp.z);
	}

	// 旋转面法线
	for (auto& face : faces) {
		glm::vec3 rotatedNormal = rotationMat * glm::vec4(face.normal.x, face.normal.y, face.normal.z, 0.0f);
		glm::vec3 temp=glm::normalize(rotatedNormal);
		face.normal =Vec3f(temp.x, temp.y, temp.z);
	}
}