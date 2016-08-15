#pragma once

#include "Controller.h"
#include "../../Engine/Graphics/Camera.h"

namespace eae6320
{
	struct FlyCam : public Controller
	{
		float yaw;
		float tracking_speed;
		float panning_speed;

		Camera fly_cam;

		FlyCam(Vector3 position, float yaw,
			float tracking_speed, float pan_speed);
		virtual ~FlyCam();

		virtual void update(Controls controls, float dt);
	};
}
