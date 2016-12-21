#include "AABB3.h"
#include "Triangle3.h"

namespace eae6320
{
	bool AABB3::contains(const AABB3 & other) const
	{
		return vmin.x <= other.vmin.x
			&& vmin.y <= other.vmin.y
			&& vmin.z <= other.vmin.z
			&& vmax.x >= other.vmax.x
			&& vmax.y >= other.vmax.y
			&& vmax.z >= other.vmax.z;
	}

	bool AABB3::intersects(const AABB3 & other) const
	{
		return vmin.x >= other.vmax.x
			&& vmin.y >= other.vmax.y
			&& vmin.z >= other.vmax.z
			&& vmax.x <= other.vmin.x
			&& vmax.y <= other.vmin.y
			&& vmax.z <= other.vmin.z;
	}

	bool AABB3::intersects(const Segment3 & ray) const
	{
		Vector3 c = (vmin + vmax) / 2; // Box center-point
		Vector3 e = vmax - c; // Box halflength extents
		Vector3 m = (ray.a + ray.b) / 2; // Segment midpoint
		Vector3 d = ray.b - m; // Segment halflength vector
		m = m - c; // Segment midpoint relative to box center

				   // Try world coordinate axes as separating axes
		Vector3 ad = d.abs();
		Vector3 check = ad + e - m.abs();
		if (check.x < 0 || check.y < 0 || check.z < 0)
			return false;

		// Add in an epsilon term to counteract arithmetic errors when segment is
		// (near) parallel to a coordinate axis
		ad += Vector3(1e-9f, 1e-9f, 1e-9f);

		// Try cross products of segment direction vector with coordinate axes
		if (fabsf(m.y * d.z - m.z * d.y) > e.y * ad.z + e.z * ad.y) return false;
		if (fabsf(m.z * d.x - m.x * d.z) > e.x * ad.z + e.z * ad.x) return false;
		if (fabsf(m.x * d.y - m.y * d.x) > e.x * ad.y + e.y * ad.x) return false;

		// No separating axis found; segment must be overlapping AABB
		return true;
	}

	AABB3 AABB3::octant(uint8_t n) const
	{
		AABB3 subbox(*this);

		Vector3 center = (vmin + vmax) / 2;

		if (n & 1) subbox.vmax.x = center.x;
		else subbox.vmin.x = center.x;
		if (n & 2) subbox.vmax.y = center.y;
		else subbox.vmin.y = center.y;
		if (n & 4) subbox.vmax.z = center.z;
		else subbox.vmin.z = center.z;

		return subbox;
	}

	AABB3 AABB3::square() const
	{
		Vector3 extents = (vmax - vmin) / 2;

		float max_extent = extents.max_dim();

		Vector3 diff = Vector3(max_extent, max_extent, max_extent) - extents;

		return AABB3(vmin - diff, vmax + diff);
	}
}