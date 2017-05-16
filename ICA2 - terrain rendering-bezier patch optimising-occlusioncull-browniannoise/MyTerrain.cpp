#include "MyTerrain.hpp"

TerrainGL::TerrainGL(int meshSizeX, int meshSizeZ, int targetSizeX, int targetSizeZ)
{
	MakeMesh(meshSizeX, meshSizeZ, targetSizeX, targetSizeZ);
}

TerrainGL::~TerrainGL()
{
}

/*
My MakeMesh function generates and interpolates a grid of a user-defined amount of vertices across a 
user-defined target world space, as well as generating global UV's to later us in bezier patch calculations

I also used the anti clockwise winding order, and a modulus toggle, to create a more optimised mesh indexing pattern, 
moving from left-incline to right-incline per triangle
*/
void TerrainGL::
MakeMesh(int meshSizeX, int meshSizeZ, int targetSizeX, int targetSizeZ)
{
	verts_x = meshSizeX + 1;
	verts_z = (float)meshSizeZ + 1;

	terrain_data.resize(verts_x * (size_t)verts_z);

	for (signed int z = 0; z < verts_z; ++z)
	{
		for (signed int x = 0; x < verts_x; ++x)
		{
			// Basic data, we reverse the z to make sure the grid is below cubes
			glm::vec3 thisVertex = glm::vec3(x * targetSizeX / verts_x, 0, -z * targetSizeZ / verts_z);

			float U = (float)x / verts_x;
			float V = (float)z / verts_z;

			// Global UV
			terrain_data[x + z * verts_x].p = thisVertex;
			terrain_data[x + z * (size_t)verts_z].globalUV = glm::vec2(U, V);
		}
	}

#pragma region ElementOptimising //Indexing optimisation
	for (int z = 0; z < (int)meshSizeZ; ++z)
	{
		int K = z * (int)verts_z;

		for (int x = 0; x < (int)meshSizeX; ++x)
		{
			if (z % 2)
			{
				if (x % 2) //even
				{
					RightIndexing(K, x, meshSizeX);
				}
				else //odd
				{
					LeftIndexing(K, x, meshSizeX);
				}
			}
			else
			{
				if (x % 2) //even
				{
					LeftIndexing(K, x, meshSizeX);
				}
				else //odd
				{
					RightIndexing(K, x, meshSizeX);
				}
			}
		}

	}

#pragma endregion

	width = meshSizeX;
	height = meshSizeZ;
}


/*
While the below function assumes a direct equality between the mesh and the miage size,
using the bezier interpolation function allows higher resolution translation of the initial heightmapped mesh
*/
void TerrainGL::
ApplyHeightMap(tygra::Image heightImage)
{
	int index = 0; //quicker than calling maths each loop to calc array position, just have a tracking index
	for (size_t z = 0; z < (int)heightImage.height(); ++z)
	{
		for (size_t x = 0; x < (int)heightImage.height(); ++x)
		{
			uint8_t height = *(uint8_t*)heightImage(x, z); //height of this pixel
			terrain_data[index].p.y = height; //store it as the y value for the corresponding vertex
			index++;
		}
	}
}

/*Get the patch of control points going upwards in ascending U-V order.
We do this rather than my old dprecated method of pushing back to a vector as it is quicker to load 
and there is no reason to store this info
*/
std::vector<glm::vec3> TerrainGL::
DefinePatches(TerrainGL* sourceMesh, bool upperPointversion, int offset)
{
	std::vector<glm::vec3> thisPatch{ 16 }; //this patches four corner control points will be in here

	thisPatch[0] = sourceMesh->terrain_data[offset].p;
	thisPatch[1] = sourceMesh->terrain_data[offset + 1].p;
	thisPatch[2] = sourceMesh->terrain_data[offset + 2].p;
	thisPatch[3] = sourceMesh->terrain_data[offset + 3].p;

	thisPatch[4] = sourceMesh->terrain_data[offset + sourceMesh->verts_x].p;
	thisPatch[5] = sourceMesh->terrain_data[offset + sourceMesh->verts_x + 1].p;
	thisPatch[6] = sourceMesh->terrain_data[offset + sourceMesh->verts_x + 2].p;
	thisPatch[7] = sourceMesh->terrain_data[offset + sourceMesh->verts_x + 3].p;

	thisPatch[8] = sourceMesh->terrain_data[offset + (sourceMesh->verts_x * 2)].p;
	thisPatch[9] = sourceMesh->terrain_data[offset + (sourceMesh->verts_x * 2) + 1].p;
	thisPatch[10] = sourceMesh->terrain_data[offset + (sourceMesh->verts_x * 2) + 2].p;
	thisPatch[11] = sourceMesh->terrain_data[offset + (sourceMesh->verts_x * 2) + 3].p;

	thisPatch[12] = sourceMesh->terrain_data[offset + (sourceMesh->verts_x * 3)].p;
	thisPatch[13] = sourceMesh->terrain_data[offset + (sourceMesh->verts_x * 3) + 1].p;
	thisPatch[14] = sourceMesh->terrain_data[offset + (sourceMesh->verts_x * 3) + 2].p;
	thisPatch[15] = sourceMesh->terrain_data[offset + (sourceMesh->verts_x * 3) + 3].p;

	return thisPatch;
}

/*PIECEWISE USING THE PATCH OFFSET MODULUS 
AGAINST 3 DUE TO HAVING ROW AND COLUMN OF 4 CPS
*/
void TerrainGL::
PieceWiseInterpolation(TerrainGL* sourceMesh)
{
	int sourceWidth = sourceMesh->verts_x;
	int percentTracker = terrain_data.size() / 100;
	int percent = 0;
	int percentIndex = 0;

	for (size_t y = 0; y < verts_z; ++y)
	{
		for (size_t x = 0; x < verts_x; ++x)
		{
			size_t offset = x + y * verts_x;

			float X = (terrain_data[offset].globalUV.x  *  ((float)sourceMesh->width));
			float Y = (terrain_data[offset].globalUV.y  *  ((float)sourceMesh->height));

			int xPatchOffset = (int)X - (int)X % 3; //acts as a patch toggle based on the 4 control points per patch X/Z axis
			int yPatchOffset = (int)Y - (int)Y % 3;

			int patchID = xPatchOffset + yPatchOffset * sourceWidth;

			float U = (X - xPatchOffset) / 3;
			float V = (Y - yPatchOffset) / 3;

			terrain_data[offset].p.y = utilAyre::BezierPatchSixteenPoints(DefinePatches(sourceMesh, true, patchID), U, V).y;

			percentIndex++;
			if (percentIndex == percentTracker)
			{
				percent++;
				std::cout << percent << "% Bezier Processed" << std::endl;
				percentIndex = 0;
			}
		}
	}

}

/*
Apply a simplistic brownian Perlin noise to the mesh's y value per vertex
I aimed for a very minor and rough detail similar to grassland
*/
void TerrainGL::
ApplyNoise()
{
	for (size_t i = 0; i < terrain_data.size(); i++)
	{
		terrain_data[i].p = utilAyre::Brownian(terrain_data[i].p, 400, 8, 1.7f, 0.65f, 1.5f);
	}
}

/*
Calculates the cross product of the traingles points in order to get the surface normal per triangle
Normalises every normal at the end
*/
void TerrainGL::
CalculateNormals()
{
	glm::vec3 tempNormal;

	for (size_t i = 0; i < terrain_elements.size(); i += 3) // for each triangle in the entire terrain
	{
		glm::vec3 p1 = terrain_data[terrain_elements[i]].p; //get the triangle positions out of the three elements
		glm::vec3 p2 = terrain_data[terrain_elements[i + 1]].p;
		glm::vec3 p3 = terrain_data[terrain_elements[i + 2]].p;

		glm::vec3 u = p2 - p1;
		glm::vec3 v = p3 - p1;

		tempNormal = glm::cross(u, v);

		terrain_data[terrain_elements[i]].n += tempNormal; // push this normal into the list
		terrain_data[terrain_elements[i + 1]].n += tempNormal; // push this normal into the list
		terrain_data[terrain_elements[i + 2]].n += tempNormal; // push this normal into the list

	}

	for (size_t i = 0; i < terrain_elements.size(); i += 3)
	{
		glm::normalize(tempNormal);
	}
}

void TerrainGL::
LeftIndexing(int K, int x, int meshSizeX)
{
	terrain_elements.push_back(K + x);
	terrain_elements.push_back(K + x + 1);
	terrain_elements.push_back(K + x + meshSizeX + 1);
	terrain_elements.push_back(K + x + 1);
	terrain_elements.push_back(K + x + meshSizeX + 2);
	terrain_elements.push_back(K + x + meshSizeX + 1);
}

void TerrainGL::
RightIndexing(int K, int x, int meshSizeX)
{
	terrain_elements.push_back(K + x);
	terrain_elements.push_back(K + x + 1);
	terrain_elements.push_back(K + x + meshSizeX + 2);
	terrain_elements.push_back(K + x);
	terrain_elements.push_back(K + x + meshSizeX + 2);
	terrain_elements.push_back(K + x + meshSizeX + 1);
}

//-------------DEPRECATED METHODS BELOW-------------------------\\

#pragma region PerlinUnused
//Perlin::Perlin(int seed) 
//{
//	p.resize(256);
//
//	std::iota(p.begin(), p.end(), 0);
//
//	// Initialize a random engine with seed
//	std::default_random_engine engine(seed);
//
//	// Suffle  using the above random engine
//	std::shuffle(p.begin(), p.end(), engine);
//
//	// Duplicate the permutation vector
//	p.insert(p.end(), p.begin(), p.end());
//}
//
//int Perlin::noise(int x, int y, int z) {
//	// Find the unit cube that contains the point
//	int X = (int)floor(x) & 255;
//	int Y = (int)floor(y) & 255;
//	int Z = (int)floor(z) & 255;
//
//	// Find relative x, y,z of point in cube
//	x -= floor(x);
//	y -= floor(y);
//	z -= floor(z);
//
//	// Compute fade curves for each of x, y, z
//	double u = fade(x);
//	double v = fade(y);
//	double w = fade(z);
//
//	// Hash coordinates of the 8 cube corners
//	int A = p[X] + Y;
//	int AA = p[A] + Z;
//	int AB = p[A + 1] + Z;
//	int B = p[X + 1] + Y;
//	int BA = p[B] + Z;
//	int BB = p[B + 1] + Z;
//
//	// Add blended results from 8 corners of cube
//	double res = lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)), lerp(u, grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z))), lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1)), lerp(u, grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1))));
//	return (res + 1.0) / 2.0;
//}
//
//double Perlin::fade(double t) {
//	return t * t * t * (t * (t * 6 - 15) + 10);
//}
//
//double Perlin::lerp(double t, double a, double b) {
//	return a + t * (b - a);
//}
//
//double Perlin::grad(int hash, double x, double y, double z) {
//	int h = hash & 15;
//	// Convert lower 4 bits of hash inot 12 gradient directions
//	double u = h < 8 ? x : y,
//		v = h < 4 ? y : h == 12 || h == 14 ? x : z;
//	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
//}
#pragma endregion

#pragma region CPS4 patching
//void TerrainGL::
//DefinePatches(TerrainGL* sourceMesh)
//{
//	std::vector<glm::vec3> thisPatch{ 4 }; //this patches four corner control points will be in here
//
//	for (size_t z = 0; z < sourceMesh->height; ++z)
//	{
//		for (size_t x = 0; x < sourceMesh->width; ++x)
//		{
//			size_t offset = x + (z * (sourceMesh->width));
//
//			thisPatch[0] = sourceMesh->terrain_data[offset].p;
//			thisPatch[1] = sourceMesh->terrain_data[offset + 1].p;
//			thisPatch[2] = sourceMesh->terrain_data[sourceMesh->verts_x + offset].p;
//			thisPatch[3] = sourceMesh->terrain_data[sourceMesh->verts_x + offset + 1].p;
//
//			terrain_patches_CPS.push_back(thisPatch);
//		}
//	}
//}
#pragma endregion
