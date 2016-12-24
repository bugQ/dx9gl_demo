#pragma once

#include "Camera.h"
#include "../Physics/Terrain.h"
#include "../Math/Vector3.h"
#include "../Math/Vector2.h"

#include "Wireframe.h"

namespace eae6320
{
namespace Graphics
{

struct FloatCamera : public Camera
{
	const float float_cam_radius = 3.0f, float_cam_height = 1.0f;
	const float tangent_x = 0.16f, tangent_y = 0.16f;
	const float tangent_speed = 10.0f, max_speed = 3.0f;

	Vector2 tangent_velocity;
	Vector3 velocity;
	const Vector3 & target;
	const Physics::Terrain & terrain;

	void update(float dt);

	void draw_debug(Wireframe & wireframe);

	FloatCamera(const Vector3 & target, const Physics::Terrain & terrain);
};

}
}
