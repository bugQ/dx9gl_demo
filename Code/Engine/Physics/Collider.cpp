#include "stdafx.h"
#include "Collider.h"

#include <limits>
#include <algorithm>

namespace eae6320
{
namespace Physics
{

bool Collider::move(Vector3 displacement, const Terrain & terrain)
{
	if (displacement == Vector3::Zero) return false;

	Vector3 checkpoints[2] = { position, position - height * Vector3::J };

	float t = std::numeric_limits<float>::infinity();
	Vector3 n;
	for (size_t i = 0; i < 2; ++i)
	{
		Vector3 p = checkpoints[i];
		Vector3 q = p + displacement;
		float t_i;
		Vector3 n_i;

		if (terrain.intersect_segment(p, q, t_i, n_i))
		{
			t = t_i;
			n = n_i;
		}
	}

	position += displacement;
	if (std::isfinite(t))
	{
		position -= (displacement.dot(n) + 0.00001f) * n;
		return true;
	}
	else return false;
}

}
}