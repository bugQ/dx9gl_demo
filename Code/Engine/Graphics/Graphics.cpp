#include "stdafx.h"

#include "Graphics.h"

void eae6320::Graphics::DrawModel(Model & model)
{
	Matrix4 local2world = Matrix4::rotation_q(model.rotation);
	local2world.vec3(3) = model.position;
	Graphics::SetEffect(*model.effect, local2world);
	Graphics::DrawMesh(*model.mesh);
}