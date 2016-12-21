#include "stdafx.h"
#include "FloatCamera.h"

namespace eae6320
{

void FloatCamera::update(float dt)
{
	Vector3 float_dir = (target - position).normalize();
	Vector3 float_dir_xz = Vector3(float_dir.x, 0, float_dir.z).normalize();
	Vector3 float_target = target - float_dir_xz * float_cam_radius + Vector3::J * float_cam_height;
	Vector3 target_disp = float_target - position;

	position += target_disp / 4;
	rotation = Versor::orientation(-float_dir);
}

FloatCamera::FloatCamera(const Vector3 & target)
	: target(target)
{
	position = target + Vector3(0, float_cam_height, float_cam_radius);
}

}