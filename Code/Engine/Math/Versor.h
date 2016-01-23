#pragma once

#include "Vector4.h"

namespace eae6320
{
	struct Versor : Vector4
	{
		Versor();
		Versor(float x, float y, float z, float w);
		Versor(Vector4 const & v);

		Versor inverse();
		Versor rotate(Versor rotation);

		static Versor rotation_x(float radians);
		static Versor rotation_y(float radians);
		static Versor rotation_z(float radians);
	};
	inline Versor operator*(Versor const & lhs, Versor const & rhs);

#include "Versor.inl"
}
