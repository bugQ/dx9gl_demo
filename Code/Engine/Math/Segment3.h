#pragma once

#include "Vector3.h"

namespace eae6320
{
	struct Segment3
	{
		Vector3 a, b;

		Segment3() {}
		Segment3(Vector3 a, Vector3 b) : a(a), b(b) {}

	};

	inline bool operator==(Segment3 const & lhs, Segment3 const & rhs)
	{
		return lhs.a == rhs.a && lhs.b == rhs.b;
	}
	inline bool operator!=(Segment3 const & lhs, Segment3 const & rhs)
	{
		return lhs.a != rhs.a && lhs.b != rhs.b;
	}
}
