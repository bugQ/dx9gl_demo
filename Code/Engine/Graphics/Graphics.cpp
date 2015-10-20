#include "stdafx.h"

#include "Graphics.h"

void eae6320::Graphics::DrawModel(Model & model)
{
	Graphics::SetEffect(*model.effect, model.position);
	Graphics::DrawMesh(*model.mesh);
}