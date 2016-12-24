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
		float float_cam_radius = 1.0f, float_cam_height = 0.5f;
		const Physics::Terrain & terrain;
		float speed;
		bool grounded;

		virtual void update(Controls controls, float dt);
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

		Player(Vector3 position, float yaw, float height,
			const Physics::Terrain & terrain, float speed);

		virtual ~Player();
	};
}
