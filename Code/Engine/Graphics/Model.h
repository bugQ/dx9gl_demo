#pragma once

#include "Mesh.h"
#include "Effect.h"
#include "../Math/Vector3.h"
#include "../Math/Versor.h"

namespace eae6320
{
namespace Graphics
{
struct Model
{
	// references only
	Mesh * const mesh;
	Effect * const effect;
	Vector3 position;
	Versor rotation;

	Model(Mesh & mesh, Effect & effect, Vector3 position = Vector3::Zero);
};
#include "Model.inl"
}
}

