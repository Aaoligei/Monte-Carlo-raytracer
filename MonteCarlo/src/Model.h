#pragma once
#ifndef __MODEL_H
#define __MODEL_H

#include <iostream>
#include <vector>
#include <string>
#include "Vec.h"

class Vertex
{
public:
	Point3f point;
	Color3f color;
	Vec3f normal;
};

class Face
{
public:
	std::vector<int> vertexIdx;
	std::vector<int> normalIdx;
	Vec3f normal;
	Color3f color;
};


class Model
{
public:
	std::vector<Vertex> vertexes;
	std::vector<Face> faces;
	std::vector<Vec3f> normals;
	Point3f center_point;

	bool load(const std::string& path);
	Model() {};
	Model(const std::string& path);
	void rotate(float** RotateMat, std::vector<Vertex>& vertexes);

};

#endif