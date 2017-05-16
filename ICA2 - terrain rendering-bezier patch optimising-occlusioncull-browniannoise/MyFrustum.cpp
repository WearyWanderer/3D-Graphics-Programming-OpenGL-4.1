#include "MyFrustum.hpp"

float FrustumPlane::getDistance(glm::vec3 pos)
{
	return glm::dot(glm::vec3(a,b,c), pos) + d;
}

MyFrustum::MyFrustum()
{
}


MyFrustum::~MyFrustum()
{
}

void MyFrustum::ConstructFrustum(float depth, glm::mat4 projMatrix, glm::mat4 viewMatrix)
{
	//float zMinimum, r;
	//glm::mat4 projCopy(projMatrix); //construct a copy as we are altering in this loop, and alterations would mess up
	glm::mat4 tempMatrix;

	//zMinimum = -projCopy[3][2] / projCopy[2][2]; //calc minimum z distance within the frame
	//r = depth / (depth - zMinimum);
	//projCopy[2][2] = r;
	//projCopy[3][2] = -r * zMinimum;

	// Create the frustum matrix from the view matrix and updated projection matrix.
	tempMatrix = projMatrix * viewMatrix; //could use glm::matrixCompMult here(?) Note to self --- test to see difference

	// Calculate near plane of frustum.
	frustumPlanes[0].a = tempMatrix[0][3] + tempMatrix[0][2];
	frustumPlanes[0].b = tempMatrix[1][3] + tempMatrix[1][2];
	frustumPlanes[0].c = tempMatrix[2][3] + tempMatrix[2][2];
	frustumPlanes[0].d = tempMatrix[3][3] + tempMatrix[3][2];

	// Calculate far plane of frustum.
	frustumPlanes[1].a = tempMatrix[0][3] - tempMatrix[0][2];
	frustumPlanes[1].b = tempMatrix[1][3] - tempMatrix[1][2];
	frustumPlanes[1].c = tempMatrix[2][3] - tempMatrix[2][2];
	frustumPlanes[1].d = tempMatrix[3][3] - tempMatrix[3][2];

	// Calculate left plane of frustum.
	frustumPlanes[2].a = tempMatrix[0][3] + tempMatrix[0][0];
	frustumPlanes[2].b = tempMatrix[1][3] + tempMatrix[1][0];
	frustumPlanes[2].c = tempMatrix[2][3] + tempMatrix[2][0];
	frustumPlanes[2].d = tempMatrix[3][3] + tempMatrix[3][0];

	// Calculate right plane of frustum.
	frustumPlanes[3].a = tempMatrix[0][3] - tempMatrix[0][0];
	frustumPlanes[3].b = tempMatrix[1][3] - tempMatrix[1][0];
	frustumPlanes[3].c = tempMatrix[2][3] - tempMatrix[2][0];
	frustumPlanes[3].d = tempMatrix[3][3] - tempMatrix[3][0];

	// Calculate top plane of frustum.
	frustumPlanes[4].a = tempMatrix[0][3] - tempMatrix[0][1];
	frustumPlanes[4].b = tempMatrix[1][3] - tempMatrix[1][1];
	frustumPlanes[4].c = tempMatrix[2][3] - tempMatrix[2][1];
	frustumPlanes[4].d = tempMatrix[3][3] - tempMatrix[3][1];

	// Calculate bottom plane of frustum.
	frustumPlanes[5].a = tempMatrix[0][3] + tempMatrix[0][1];
	frustumPlanes[5].b = tempMatrix[1][3] + tempMatrix[1][1];
	frustumPlanes[5].c = tempMatrix[2][3] + tempMatrix[2][1];
	frustumPlanes[5].d = tempMatrix[3][3] + tempMatrix[3][1];

	return;
}

bool MyFrustum::IsPointOnScreen(glm::vec3 pos)
{
	for (unsigned int i = 0; i < 6; ++i) //for each of the planes, check if the point is within 
	{
		if (frustumPlanes[i].getDistance(pos) < 0.0f) //if the point is below the plane and as such not on the screen
		{
			return false;
		}
	}
	return true;
}
