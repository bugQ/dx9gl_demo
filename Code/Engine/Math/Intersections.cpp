#include "Intersections.h"

namespace eae6320
{
namespace Intersections
{

// Given segment pq and triangle abc, returns whether segment intersects
// triangle and if so, also returns the barycentric coordinates (u,v,w)
// of the intersection point
bool SegmentTriangle(Vector3 p, Vector3 q, Vector3 a, Vector3 b, Vector3 c,
	float &u, float &v, float &w, float &t)
{
	Vector3 ab = b - a;
	Vector3 ac = c - a;
	Vector3 qp = p - q;

	// Compute triangle normal. Can be precalculated or cached if
	// intersecting multiple segments against the same triangle
	Vector3 n = ab.cross(ac);

	// Compute denominator d. If d <= 0, segment is parallel to or points
	// away from triangle, so exit early
	float d = qp.dot(n);
	if (d <= 0.0f) return false;

	// Compute intersection t value of pq with plane of triangle. A ray
	// intersects if 0 <= t. Segment intersects if 0 <= t <= 1. Delay
	// dividing by d until intersection has been found to pierce triangle
	Vector3 ap = p - a;
	t = ap.dot(n);
	if (t < 0.0f) return false;
	if (t > d) return false; // For segment; exclude this code line for a ray test

							 // Compute barycentric coordinate components and test if within bounds
	Vector3 e = qp.cross(ap);
	v = ac.dot(e);
	if (v < 0.0f || v > d) return false;
	w = -ab.dot(e);
	if (w < 0.0f || v + w > d) return false;

	// Segment/ray intersects triangle. Perform delayed division and
	// compute the last barycentric coordinate component
	float ood = 1.0f / d;
	t *= ood;
	v *= ood;
	w *= ood;
	u = 1.0f - v - w;
	return true;
}

}
}