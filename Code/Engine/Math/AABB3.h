#pragma once

#include "Vector3.h"

namespace eae6320
{
	struct AABB3
	{
		Vector3 vmin, vmax;

		AABB3() {}
		AABB3(const AABB3 & other) : vmin(other.vmin), vmax(other.vmax) {}
		AABB3(Vector3 vmin, Vector3 vmax) : vmin(vmin), vmax(vmax) {}
		~AABB3() {}

		AABB3 scale(const Vector3 & rhs) const
		{
			return AABB3(vmin.scale(rhs), vmax.scale(rhs));
		}

		bool contains(const AABB3 &) const;
		bool intersects(const AABB3 &) const;

		AABB3 octant(uint8_t n) const;
	};
}