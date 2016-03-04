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

	Sprite(Material * material, Rect xy, Rect uv = { 0, 0, 1, 1 })
		: mat(material), xy(xy), uv(uv)
	{
	}

	~Sprite();
};
}
}