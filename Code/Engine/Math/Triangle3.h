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
			normal = (b - a).cross(c - a);
		}
		~Triangle3() {}

		bool intersect_segment(Vector3 p, Vector3 q, float &t, Vector3 &n) const;
	};
}
