#pragma once
#include "../../Engine/Graphics/Camera.h"
#include "../../Engine/Physics/Collider.h"
#include "Controller.h"

namespace eae6320
{
	struct Player : public Physics::Collider, public Controller
	{
		Camera head_cam;
		const Physics::Terrain & terrain;
		float speed;

		virtual void update(Controls controls, float dt);
		void update_cam();

		Player(Vector3 position, float yaw, float height,
			const Physics::Terrain & terrain, float speed);

		virtual ~Player();
	};
}
