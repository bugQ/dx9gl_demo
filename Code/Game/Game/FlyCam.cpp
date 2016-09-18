#include "FlyCam.h"

namespace eae6320
{

void FlyCam::update(Controls controls, float dt)
{
	yaw += -controls.joy_right.x * panning_speed * dt;
	fly_cam.rotation = Versor::rotation_y(yaw);
	Vector3 dir(controls.joy_left.x, controls.joy_right.y, -controls.joy_left.y);
	dir = fly_cam.rotation.rotate(dir);
	fly_cam.position += dir * tracking_speed * dt;
}

FlyCam::FlyCam(Vector3 position, float yaw, float tracking_speed, float panning_speed)
	: fly_cam({ position, Versor::rotation_y(yaw) }), yaw(yaw), tracking_speed(tracking_speed), panning_speed(panning_speed)
{
}


FlyCam::~FlyCam()
{
}

}
