#include "Player.h"

namespace eae6320
{

Player::Player(Vector3 position, float yaw, float height, const Physics::Terrain & terrain, float speed)
	: Collider(position, yaw, height), terrain(terrain), speed(speed), grounded(false)
{
	update_cam();
}

void Player::update(Controls controls, float dt)
{
	yaw += dt * -controls.joy_right.x;
	if (grounded && controls.joy_right.y > 0) {
		velocity.y = speed;
		grounded = false;
	}
	velocity.y -= speed * dt;
	Vector3 dir(controls.joy_left.x, 0, -controls.joy_left.y);
	dir = head_cam.rotation.rotate(dir);
	Vector3 prev = position;
	grounded = move((dir * speed + velocity) * dt, terrain);
	if (grounded)
		velocity.y = 0;
	update_cam();
}

void Player::update_cam()
{
	head_cam.position = position;
	head_cam.rotation = Versor::rotation_y(yaw);
}

Player::~Player()
{
}
}