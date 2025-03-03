#include "Model.h"
#include <fstream>
#include <iostream>
#include <string>
#include <ctime>

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

void Model::rotate(float** RotateMat, vector<Vertex>& vertexes)
{
	int vertex_num = vertexes.size();
#pragma omp parallel for
	for (int i = 0; i < vertex_num; ++i)
	{

		Point3f point;
		Point3f tmp_point = vertexes[i].point - center_point;
		vertexes[i].point.x = RotateMat[0][0] * tmp_point.x + RotateMat[0][1] * tmp_point.y + RotateMat[0][2] * tmp_point.z;
		vertexes[i].point.y = RotateMat[1][0] * tmp_point.x + RotateMat[1][1] * tmp_point.y + RotateMat[1][2] * tmp_point.z;
		vertexes[i].point.z = RotateMat[2][0] * tmp_point.x + RotateMat[2][1] * tmp_point.y + RotateMat[2][2] * tmp_point.z;
		vertexes[i].point += center_point;
	}

	int normal_num = normals.size();
#pragma omp parallel for
	for (int i = 0; i < normal_num; ++i)
	{
		Point3f tmp_point = normals[i];
		normals[i].x = RotateMat[0][0] * tmp_point.x + RotateMat[0][1] * tmp_point.y + RotateMat[0][2] * tmp_point.z;
		normals[i].y = RotateMat[1][0] * tmp_point.x + RotateMat[1][1] * tmp_point.y + RotateMat[1][2] * tmp_point.z;
		normals[i].z = RotateMat[2][0] * tmp_point.x + RotateMat[2][1] * tmp_point.y + RotateMat[2][2] * tmp_point.z;

	}

	int face_num = faces.size();
#pragma omp parallel for
	for (int i = 0; i < face_num; ++i)
	{
		Point3f tmp_point = faces[i].normal;
		faces[i].normal.x = RotateMat[0][0] * tmp_point.x + RotateMat[0][1] * tmp_point.y + RotateMat[0][2] * tmp_point.z;
		faces[i].normal.y = RotateMat[1][0] * tmp_point.x + RotateMat[1][1] * tmp_point.y + RotateMat[1][2] * tmp_point.z;
		faces[i].normal.z = RotateMat[2][0] * tmp_point.x + RotateMat[2][1] * tmp_point.y + RotateMat[2][2] * tmp_point.z;
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