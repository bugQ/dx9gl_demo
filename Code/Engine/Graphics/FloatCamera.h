#pragma once

#include "Camera.h"
#include "../Physics/Terrain.h"
#include "../Math/Vector3.h"
#include "../Math/Vector2.h"

#include "Wireframe.h"

#include <array>

namespace eae6320
{
namespace Graphics
{

struct FloatCamera : public Camera
{

	Vector2 tangent_velocity;
	Vector3 velocity;

	std::deque<Vector3> position_buffer;

	const Vector3 & target;

	void update(Physics::Terrain & terrain, float dt);

	FloatCamera(const Vector3 & target);
};

}
}
