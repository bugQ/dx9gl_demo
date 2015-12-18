#include "stdafx.h"

#include "Graphics.h"

void eae6320::Graphics::DrawModel(Model & model, Camera & camera)
{
	Matrix4 local2world = Matrix4::rotation_q(model.rotation);
	local2world.vec3(3) = model.position;
	Graphics::SetMaterial(*model.mat);
	Graphics::SetTransform(*model.mat->effect, local2world);
	Graphics::SetCamera(*model.mat->effect, camera);
	Graphics::DrawMesh(*model.mesh);
}
