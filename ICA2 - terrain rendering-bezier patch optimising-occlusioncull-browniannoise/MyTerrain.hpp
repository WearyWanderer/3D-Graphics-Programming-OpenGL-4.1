#pragma once

#include <vector>
#include <iostream>
#include <numeric>
#include <random>
#include <algorithm>
#include <glm\glm.hpp>
#include <tygra\Image.hpp>
#include "NoiseBezierLib.hpp" //include my perlin noise and bezier library

struct Perlin
{
	Perlin(int seed);
	std::vector<int> p;
	int noise(int x, int y, int z);
	double fade(double t);
	double lerp(double t, double a, double b);
	double grad(int hash, double x, double y, double z);
}; //UNUSED

/*
Created a structure to hold the various information that I need to call in multiple functions per-vertex. 
*/
struct Vertex
{
	glm::vec3 p;
	glm::vec3 n;
	glm::vec2 globalUV;
	glm::vec2 localUV;
};

class TerrainGL
{
public:
	TerrainGL(int meshSizeX, int meshSizeZ, int targetSizeX, int targetSizeZ);
	~TerrainGL();

	std::vector<Vertex> terrain_data;
	std::vector<int> terrain_elements;
	size_t width, height;
	size_t verts_x;
	float verts_z; //this has to be a float for some visual calculations

	void
	MakeMesh(int meshSizeX, int meshSizeZ, int targetSizeX, int targetSizeZ);

	std::vector<glm::vec3> TerrainGL::
	DefinePatches(TerrainGL* sourceMesh, bool upperPointversion, int offset);

	void
	ApplyHeightMap(tygra::Image heightImage);

	void
	PieceWiseInterpolation(TerrainGL* sourceMesh);

	void
	BezierInterpolation(TerrainGL* sourceMesh);

	void 
	DoTheBezier();

	void
	ApplyNoise();

	void
	CalculateNormals();

	void 
	LeftIndexing(int K, int x, int meshSizeX);

	void 
	RightIndexing(int K, int x, int meshSizeX);
};

