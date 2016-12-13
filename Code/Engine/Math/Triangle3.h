#pragma once
#include "Vector3.h"
#include "AABB3.h"

namespace eae6320
{
	struct Triangle3
	{
		Vector3 a, b, c, normal;
		AABB3 box;

		Triangle3() {}
		Triangle3(const Triangle3 & other) { *this = other; }
		Triangle3(Vector3 a, Vector3 b, Vector3 c, Vector3 normal);
		Triangle3(Vector3 a, Vector3 b, Vector3 c)
			: Triangle3(a, b, c, (b - a).cross(c - a).normalize()) {}
		~Triangle3() {}

		float intersect_ray(Vector3 p, Vector3 q) const;

		Triangle3 scale(const Vector3 & rhs) const
		{
			Triangle3 copy(*this);
			copy.a = copy.a.scale(rhs);
			copy.b = copy.b.scale(rhs);
			copy.c = copy.c.scale(rhs);
			copy.box = copy.box.scale(rhs);
			return copy;
		}
	};
}
