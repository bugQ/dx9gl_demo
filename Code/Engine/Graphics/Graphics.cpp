#include "stdafx.h"

#include "Graphics.h"

void eae6320::Graphics::DrawWireframe(Wireframe & wireframe, Camera & camera)
{
	eae6320::Graphics::BufferWireframe(wireframe);

	Graphics::SetMaterial(*wireframe.material);
	Graphics::SetTransform(*wireframe.material->effect, Matrix4::Identity);
	Graphics::SetCamera(*wireframe.material->effect, camera);
	Graphics::DrawWireMesh(*wireframe.mesh);
}

void eae6320::Graphics::DrawModel(Model & model, Camera & camera)
{
	Matrix4 local2world = Matrix4::rotation_q(model.rotation);
	local2world = local2world.dot(Matrix4::scale(model.scale));
	local2world.vec3(3) = model.position;
	Graphics::SetMaterial(*model.mat);
	Graphics::SetTransform(*model.mat->effect, local2world);
	Graphics::SetCamera(*model.mat->effect, camera);
	Graphics::DrawMesh(*model.mesh);
}

void eae6320::Graphics::SetMaterial(Material & material)
{
	SetEffect(*material.effect);
	material.SetParams();
}