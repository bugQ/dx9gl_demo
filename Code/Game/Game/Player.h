#pragma once
#include "../../Engine/Graphics/Camera.h"
#include "../../Engine/Graphics/FloatCamera.h"
#include "../../Engine/Physics/Collider.h"
#include "Controller.h"
#include "../../Engine/Graphics/Wireframe.h"

namespace eae6320
{
	struct Player : public Physics::Collider, public Controller
	{
		Graphics::Camera head_cam;
		Graphics::FloatCamera float_cam;
		float speed;
		bool grounded;

		Physics::Terrain * terrain = NULL;
		void(*update_callback)() = NULL;
		const float callback_interval = 0.1f;
		float callback_timer = callback_interval;

		virtual void update(Controls controls, float dt);
		void remote_update(const Vector3 & pos, float rot);
		void update_cam();

		void draw_debug(Graphics::Wireframe & wireframe)
#ifdef _DEBUG
		;
#else
		{}
#endif

		Segment3 line_of_sight()
		{
			return Segment3(head_cam.position, head_cam.rotation.rotate(-Vector3::K) * 100);
		}

		Player(Vector3 position, float yaw, float height = 1.5f, float speed = 2.0f);

		virtual ~Player();
	};
}
