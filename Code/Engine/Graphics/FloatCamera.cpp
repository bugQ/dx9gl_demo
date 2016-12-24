#include "stdafx.h"
#include "FloatCamera.h"

namespace eae6320
{
namespace Graphics
{

	const float float_cam_radius = 3.0f, float_cam_height = 1.0f;
	const float tangent_x = 0.32f, tangent_y = 0.16f;
	const float tangent_speed = 10.0f, max_speed = 3.0f;
	const size_t buffer_length = 10;

void FloatCamera::update(float dt)
{
	Vector3 float_dir = (target - position).normalize();
	Vector3 float_dir_xz = Vector3(float_dir.x, 0, float_dir.z).normalize();
	Vector3 float_target = target - float_dir_xz * float_cam_radius + Vector3::J * float_cam_height;
	Vector3 target_disp = float_target - position;

	velocity += target_disp.clip(max_speed);
	
	Vector3 forward = -float_dir;
	Vector3 up = Vector3::J;
	Vector3 right;
	rotation = Versor::orientation(forward, up, right);

	up *= tangent_y;
	right *= tangent_x;
	Vector3 offset = position - target;
	Vector3 up_right = up + right;
	Vector3 up_left = up - right;
	Vector3 down_right = up + right;
	Vector3 down_left = up - right;

	float t_up = fminf(1, terrain.intersect_ray(target + up, offset + up));
	float t_down = fminf(1, terrain.intersect_ray(target - up, offset - up));
	float t_right = fminf(1, terrain.intersect_ray(target + right, offset + right));
	float t_left = fminf(1, terrain.intersect_ray(target - right, offset - right));

	tangent_velocity.x += tangent_speed * (t_right - t_left);
	tangent_velocity.y += tangent_speed * (t_up - t_down);

	tangent_velocity.clip(tangent_speed);
	velocity += rotation.rotate(Vector3(tangent_velocity.x, tangent_velocity.y, 0));

	position_buffer.pop_front();
	position_buffer.push_back(position_buffer.back() + dt * velocity.clip(max_speed));

	Vector3 pos_sum = Vector3::Zero;
	for (Vector3 & v : position_buffer)
		pos_sum += v;
	position = pos_sum / position_buffer.size();

	tangent_velocity = Vector2::Zero;
	velocity /= 2;
}

void FloatCamera::draw_debug(Wireframe & wireframe)
{
	/*
	Vector3 forward = rotation.rotate(-Vector3::K);
	Vector3 up = Vector3::J;
	Vector3 right;
	Versor::orientation(forward, up, right);

	Vector3 units(forward.norm(), up.norm(), right.norm());

	Color white(1, 1, 1, 1);
	wireframe.addLine(position, white, position + up, white);
	wireframe.addLine(position, white, position + right, white);
	wireframe.addLine(position, white, position + forward, white);

	up *= tangent_y;
	right *= tangent_x;
	Vector3 corner_up_right = position + up + right;
	Vector3 corner_up_left = position + up - right;
	Vector3 corner_down_right = position - up + right;
	Vector3 corner_down_left = position - up - right;

	Color cyan(0, 1, 1, 1);
	wireframe.addLine(position, cyan, corner_up_right, cyan);
	wireframe.addLine(position, cyan, corner_up_left, cyan);
	wireframe.addLine(position, cyan, corner_down_right, cyan);
	wireframe.addLine(position, cyan, corner_down_left, cyan);

	float t_up_right = fminf(1, terrain.intersect_ray(target, corner_up_right - target));
	float t_up_left = fminf(1, terrain.intersect_ray(target, corner_up_left - target));
	float t_down_right = fminf(1, terrain.intersect_ray(target, corner_down_right - target));
	float t_down_left = fminf(1, terrain.intersect_ray(target, corner_down_left - target));

	Vector3 hit_up_right = corner_up_right * t_up_right + target * (1 - t_up_right);
	Vector3 hit_up_left = corner_up_left * t_up_left + target * (1 - t_up_left);
	Vector3 hit_down_right = corner_down_right * t_down_right + target * (1 - t_down_right);
	Vector3 hit_down_left = corner_down_left * t_down_left + target * (1 - t_down_left);

	Color green(0, 1, 0, 1);
	Color red(1, 0, 0, 1);

	wireframe.addLine(target, green, hit_up_right, green);
	wireframe.addLine(hit_up_right, red, corner_up_right, red);
	wireframe.addLine(target, green, hit_up_left, green);
	wireframe.addLine(hit_up_left, red, corner_up_left, red);
	wireframe.addLine(target, green, hit_down_right, green);
	wireframe.addLine(hit_down_right, red, corner_down_right, red);
	wireframe.addLine(target, green, hit_down_left, green);
	wireframe.addLine(hit_down_left, red, corner_down_left, red);
	*/
}

FloatCamera::FloatCamera(const Vector3 & target, const Physics::Terrain & terrain)
	: Camera(target + Vector3(0, float_cam_height, float_cam_radius))
	, target(target), terrain(terrain), tangent_velocity(0, 0), velocity(0, 0, 0)
	, position_buffer(buffer_length, position)
{
}

}
}