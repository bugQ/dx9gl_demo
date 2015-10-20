#include "Actor.h"

namespace eae6320
{
	Actor::Actor(Mesh & mesh, Effect & effect, Vector3 position)
	{
		this->mesh = &mesh;
		this->effect = &effect;
		this->position = position;
	}
}