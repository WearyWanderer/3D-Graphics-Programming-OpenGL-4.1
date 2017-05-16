#pragma once

#include <glm\glm.hpp>

/*
I used my own plane struct to keep simplicity and a low cost to memory
*/
struct FrustumPlane
{
	float a, b, c, d;

	void normalise();
	float getDistance(glm::vec3 pos);
};

/*
Barebones Frustum class that is simple called each frame to construct a current view frustum and test the visibility of each cube on the map
*/
class MyFrustum
{
public:
	MyFrustum();
	~MyFrustum();

	void ConstructFrustum(float depth, glm::mat4 projMatrix, glm::mat4 viewMatrix);

	bool IsPointOnScreen(glm::vec3 pos);

private:

	FrustumPlane frustumPlanes[6];
};

