#pragma once

#include "Camera.h"
#include "../Math/Vector3.h"

namespace eae6320
{

struct FloatCamera : public Camera
{
	float float_cam_radius = 3.0f, float_cam_height = 1.0f;
	const Vector3 & target;

	void update(float dt);

	FloatCamera(const Vector3 & target);
};

}
