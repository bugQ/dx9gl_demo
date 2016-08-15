#pragma once

#include "../../Engine/Math/Vector2.h"

namespace eae6320
{
class Controller
{
public:
	struct Controls
	{
		Vector2 joy_left;
		Vector2 joy_right;
	};

	virtual void update(Controls controls, float dt) = 0;

	Controller();
	virtual ~Controller();
};
}
