#pragma once

#include "Material.h"

namespace eae6320
{
namespace Graphics
{
struct Sprite
{
	struct Rect
	{
		float x0, y0, x1, y1;
	};

	Rect xy, uv;
	Material * const mat;
	bool active;

	Sprite(Material & material, Rect xy, Rect uv = { 0, 0, 1, 1 }, bool active = true)
		: mat(&material), xy(xy), uv(uv), active(active)
	{
	}

	~Sprite()
	{
	}
};
}
}