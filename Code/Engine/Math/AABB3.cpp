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
}