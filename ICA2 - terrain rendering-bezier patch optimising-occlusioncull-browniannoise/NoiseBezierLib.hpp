#pragma once
#include <cmath>
#include <glm\glm.hpp>
#include <vector>

namespace utilAyre
{
	float SeededRandomFloat(int x, int y);

	float Noise(int n);

	float CosineInterpolation(float a, float b, float x);

	float PerlinNoise(int xPos, int zPos);

	glm::vec3 Brownian(const glm::vec3& pos, float frequency, int octaves, float lacunarity, float gain, float scale);

	glm::vec3 CalculateBezier(const std::vector<glm::vec3>& cps, float u, float v);

	glm::vec3 BezierSurface(const std::vector<std::vector<glm::vec3>>& cps, float u, float v, int patchID);

	glm::vec3 BezierSurface(const std::vector<glm::vec3>& cps, float u, float v);

	glm::vec3 BezierPatchSixteenPoints(const std::vector<glm::vec3>& cps, float u, float v);
}