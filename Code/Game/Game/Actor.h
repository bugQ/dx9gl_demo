#pragma once
#include "../../Engine/Graphics/Mesh.h"
#include "../../Engine/Graphics/Effect.h"

namespace eae6320
{
	class Actor
	{
		// references only
		Mesh * mesh;
		Effect * effect;



	public:
		Actor();
		~Actor();
	};
}
