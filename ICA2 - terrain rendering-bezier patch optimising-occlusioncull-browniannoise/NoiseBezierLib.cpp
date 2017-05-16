#include "NoiseBezierLib.hpp"


/*
My utility functions are for the majority of the mathematical calculations 
that are the majority of work in both the Bezier surfaces and the Perlin noise
*/
namespace utilAyre
{
	float SeededRandomFloat(int x, int y)
	{
		int n = x + y * 57;
		n = (n << 13) ^ n;
		int nn = (n*(n*n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
		return 1.0 - ((float)nn / 1073741824.0);
	}

	float Noise(int n)
	{
		n = (n >> 13) ^ n;
		int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
		return 1.0 - ((float)nn / 1073741824.0);
	}

	float CosineInterpolation(float a, float b, float x)
	{
		float ft = x * 3.1415927;
		float f = (1.0 - cos(ft))* 0.5;
		return a*(1.0 - f) + b*f;
	}

	float PerlinNoise(int xPos, int zPos)
	{
		float s, t, u, v;
		s = SeededRandomFloat(xPos, zPos);
		t = SeededRandomFloat(xPos + 1, zPos);
		u = SeededRandomFloat(xPos, zPos + 1);
		v = SeededRandomFloat(xPos + 1, zPos + 1);
		double int1 = CosineInterpolation(s, t, xPos);//Interpolate between the values.
		double int2 = CosineInterpolation(u, v, xPos);//Here we use x-xPos, to get 1st dimension. Don't mind the x-xPos thingie, it's part of the cosine formula.

		return CosineInterpolation(int1, int2, zPos);//Here we use y-yPos, to get the 2nd dimension.

	}

	glm::vec3 Brownian(const glm::vec3& pos, float frequency, int octaves, float lacunarity, float gain, float scale)
	{
		double total = 0.0f;
		float amplitude = gain;

		for (size_t i = 0; i < octaves; ++i)
		{
			total += PerlinNoise((float)pos.x * frequency, (float)pos.z * frequency) * amplitude;
			frequency *= lacunarity;
			amplitude *= gain;
		}

		return glm::vec3(pos.x, pos.y + (total * scale), pos.z);
	}

	glm::vec3 CalculateBezier(const std::vector<glm::vec3>& cps, float t)
	{
		float temps[4];

		temps[0] = (1 - t) * (1 - t) * (1 - t); //pow() unecessary overhead so do not call
		temps[1] = 3 * t * (1 - t) * (1 - t);
		temps[2] = 3 * (1 - t) * t * t;
		temps[3] = t * t * t;

		return (
			cps[0] * temps[0] +
			cps[1] * temps[1] +
			cps[2] * temps[2] +
			cps[3] * temps[3]
			);
	}

	glm::vec3 BezierPatchSixteenPoints(const std::vector<glm::vec3>& cps, float u, float v)
	{
		std::vector<glm::vec3> Pu{ 4 };
		// compute 4 control points along u direction
		for (int i = 0; i < 4; ++i)
		{
			std::vector<glm::vec3> curveP{ 4 };
			curveP[0] = cps[i * 4];
			curveP[1] = cps[i * 4 + 1];
			curveP[2] = cps[i * 4 + 2];
			curveP[3] = cps[i * 4 + 3];
			Pu[i] = CalculateBezier(curveP, u);
		}
		// compute final position on the surface using v
		return CalculateBezier(Pu, v);
	}

//----------------------------DEPRECATED BEZIER FUNCTIONS BELOW --------------------------

	glm::vec3 BezierSurface(const std::vector<std::vector<glm::vec3>>& cps, float u, float v, int patchID)
	{
		std::vector<glm::vec3> curve{ cps.size() };
		if (patchID == cps.size())
			patchID--;
		for (int j = 0; j < cps[patchID].size(); ++j)
		{
			curve[j] = CalculateBezier(cps[patchID], u);
		}

		return CalculateBezier(curve, v);
	}

	glm::vec3 BezierSurface(const std::vector<glm::vec3>& cps, float u, float v)
	{
		std::vector<glm::vec3> curve{ cps.size() };
		for (int j = 0; j < cps.size(); ++j)
		{
			curve[j] = CalculateBezier(cps, u);
		}

		return CalculateBezier(curve, v);
	}

}