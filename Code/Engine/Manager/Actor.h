#pragma once

#include "../Graphics/Mesh.h"
#include "../Graphics/Effect.h"
#include "Vector3.h"

namespace eae6320
{
	class Actor
	{
		// references only
		Mesh * mesh;
		Effect * effect;

	public:
		Vector3 position;

		Actor(Mesh & mesh, Effect & effect, Vector3 position = Vector3::Zero);
	};
}
