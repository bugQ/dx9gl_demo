#pragma once

#include "Vector3.h"

namespace eae6320
{
namespace Intersections
{
	// Given segment pq and triangle abc, returns whether segment intersects
	// triangle and if so, also returns the barycentric coordinates (u,v,w)
	// of the intersection point
	bool SegmentTriangle(Vector3 p, Vector3 q, Vector3 a, Vector3 b, Vector3 c,
		float &u, float &v, float &w, float &t);
}
}