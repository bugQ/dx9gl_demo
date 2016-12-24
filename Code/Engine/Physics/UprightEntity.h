#pragma once

#include "../Graphics/Camera.h"
#include "Terrain.h"

namespace eae6320
{
namespace Physics
{
	struct UprightEntity
	{
		Vector3 position;
		float yaw;

		Graphics::Camera perspective()
		{
			return { position, Versor::rotation_y(yaw) };
		}

		UprightEntity(Vector3 position, float yaw)
			: position(position), yaw(yaw) {}
		virtual ~UprightEntity() {}
	};
}
}
