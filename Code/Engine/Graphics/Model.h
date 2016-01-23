#pragma once

#include "Mesh.h"
#include "Material.h"
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
	Material * const mat;
	Vector3 position;
	Versor rotation;
	Vector3 scale;

	Model(Mesh & mesh, Material & mat, Vector3 position = Vector3::Zero, Vector3 scale = Vector3::One);
};
#include "Model.inl"
}
}

