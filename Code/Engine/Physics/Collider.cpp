#include "stdafx.h"
#include "Collider.h"

#include <limits>
#include <algorithm>

namespace eae6320
{
namespace Physics
{

bool Collider::move(Vector3 s, const Terrain & terrain)
{
	if (s == Vector3::Zero) return false;

	float t = std::numeric_limits<float>::infinity();
	float d = s.norm();
	Vector3 n;
	bool grounded = false;

	Vector3 o = position - height * Vector3::J;
	/**
	if (s != Vector3::Zero)
	{
		t = terrain.intersect_ray(o, s, &n);
		if (t > 0 && t <= 1)
			s -= s.dot(n) * n;
	}
	/*/
	if (s.x != 0)
	{
		t = terrain.intersect_ray(o, Vector3(s.x, 0, 0), &n);
		if (t > 0 && t <= 1)
			s -= s.dot(n) * n;// *(1 - t + 1e-5f);
	}
	if (s.z != 0)
	{
		t = terrain.intersect_ray(o, Vector3(0, 0, s.z), &n);
		if (t > 0 && t <= 1)
			s -= s.dot(n) * n;// *(1 - t + 1e-5f);
	}
	if (s.y != 0)
	{
		t = terrain.intersect_ray(position, Vector3(0, s.y - height, 0), &n);
		if (t > 0 && t <= 1) {
			s -= s.dot(n) * n;// *(1 - t + 1e-5f);
			grounded = true;
		}
	}
	/**/
	position += s;
	return grounded;
}

}
}