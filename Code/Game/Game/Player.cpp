#include "Player.h"

namespace eae6320
{

Player::Player(Vector3 position, float yaw, float height, const Physics::Terrain & terrain, float speed)
	: Collider(position, yaw, height), terrain(terrain), speed(speed)
{
	update_cam();
}

void Player::update(Controls controls, float dt)
{
	yaw += dt * controls.joy_right.x;
	Vector3 dir(controls.joy_left.x, 0, controls.joy_left.y);
	dir = head_cam.rotation.inverse().rotate(dir);
	move(dir * speed * dt, terrain);
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