#pragma once
#include "Vector3.h"

namespace eae6320
{
	struct Triangle3
	{
		Vector3 a, b, c, normal;

		Triangle3() {}
		Triangle3(const Triangle3 & other) { *this = other; }
		Triangle3(Vector3 a, Vector3 b, Vector3 c)
			: a(a), b(b), c(c)
		{
			normal = (b - a).cross(c - a).normalize();
		}
		~Triangle3() {}

		float intersect_ray(Vector3 p, Vector3 q) const;
	};
}
