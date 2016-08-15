#include "Triangle3.h"
#include <limits>

namespace eae6320
{

// Given segment pq and triangle abc, returns whether segment intersects
// triangle and if so, returns parameter.
// Otherwise, returns infinity.
bool Triangle3::intersect_segment(Vector3 p, Vector3 q, float &t, Vector3 &n) const
{
	// Compute denominator d. If d <= 0, segment is parallel to or points
	// away from triangle, so exit early
	Vector3 qp = p - q;
	float d = qp.dot(normal);
	if (d <= 0.0f) return false;

	// Compute intersection t value of pq with plane of triangle. A ray
	// intersects if 0 <= t. Segment intersects if 0 <= t <= 1. Delay
	// dividing by d until intersection has been found to pierce triangle
	Vector3 ap = p - a;
	t = ap.dot(normal);
	if (t < 0.0f) return false;
	if (t > d) return false;

	// Compute barycentric coordinate components and test if within bounds
	Vector3 e = qp.cross(ap);
	float v = (a - c).dot(e);
	if (v < 0.0f || v > d) return false;
	float w = (a - b).dot(e);
	if (w < 0.0f || v + w > d) return false;

	t /= d;
	n = normal;
	n.normalize();
	return true;
}

}