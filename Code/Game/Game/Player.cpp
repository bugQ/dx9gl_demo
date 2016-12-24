#include "Player.h"

namespace eae6320
{

Player::Player(Vector3 position, float yaw, float height, const Physics::Terrain & terrain, float speed)
	: Collider(position, yaw, height), terrain(terrain), speed(speed), grounded(false), float_cam(this->position, terrain)
{
	update_cam();
}

void Player::update(Controls controls, float dt)
{
	//yaw += dt * -controls.joy_right.x;

	// jumping
	if (grounded && controls.joy_right.y > 0) {
		velocity.y = speed;
		grounded = false;
	}
	// gravity
	velocity.y -= speed * dt;

	// movement & collision
	Vector3 joy_dir(controls.joy_left.x, 0, -controls.joy_left.y);
	Vector3 dir = Vector3::Zero;

	if (joy_dir != Vector3::Zero)
	{
		Vector3 target_dir = float_cam.rotation.rotate(joy_dir);
		target_dir.y = 0;
		target_dir.normalize();
		float target_yaw = -atan2f(target_dir.x, -target_dir.z);
		float pi = 3.1415926f, tau = pi * 2;
		float yaw_diff = target_yaw - yaw;
		if (yaw_diff < 0) yaw_diff += tau;
		if (yaw_diff < pi)
			yaw += fminf(yaw_diff, speed * pi * dt);
		else
			yaw -= fminf(tau - yaw_diff, speed * pi * dt);
		update_cam();
		dir = head_cam.rotation.rotate(-Vector3::K);
	}

	Vector3 prev = position;
	grounded = move((dir * speed + velocity) * dt, terrain);
	if (grounded)
		velocity.y = 0;

	// heading
	Vector3 disp = position - prev;
	disp.y = 0;
	if (dir.z != 0 && dir.x != 0)
	{
		yaw = -atan2f(dir.x, -dir.z);
	}

	// cameras
	update_cam();
	float_cam.tangent_velocity.x -= controls.joy_right.x * speed;
	float_cam.update(dt);
}

void Player::update_cam()
{
	head_cam.position = position;
	head_cam.rotation = Versor::rotation_y(yaw);
}

#ifdef _DEBUG
void Player::draw_debug(Graphics::Wireframe & wireframe)
{
	wireframe.addSphere(position - Vector3::J * (height / 3), height / 4, 8, Graphics::Color::White);
	wireframe.addSphere(position - Vector3::J * (2 * height / 3), height / 3, 8, Graphics::Color::White);
	wireframe.addSphere(position, height / 5, 8, Graphics::Color::White);

	Vector3 dir = head_cam.rotation.rotate(-Vector3::K);
	Vector3 perp = dir.cross(Vector3::J);
	Vector3 carrot_base = position + dir * (height / 5);
	Vector3 carrot_tip = position + dir * (height / 2);
	Graphics::Color carrot_orange(1.0f, 0.7f, 0.2f, 1.0f);

	wireframe.addLine(carrot_base + Vector3::J * (height / 16), carrot_orange, carrot_tip, carrot_orange);
	wireframe.addLine(carrot_base + perp * (height / 16), carrot_orange, carrot_tip, carrot_orange);
	wireframe.addLine(carrot_base - perp * (height / 16), carrot_orange, carrot_tip, carrot_orange);

	float_cam.draw_debug(wireframe);
}
#endif

Player::~Player()
{
}
}