#pragma once

#include "Vector4.h"

namespace eae6320
{
	struct Versor : Vector4
	{
		Versor();
		Versor(float x, float y, float z, float w);
		Versor(Vector4 const & v);

		Versor inverse() const;
		Versor rotate_by(Versor rotation) const;
		Vector3 rotate(Vector3 direction) const;

		static Versor rotation_x(float radians);
		static Versor rotation_y(float radians);
		static Versor rotation_z(float radians);

		static Versor orientation(const Vector3 & forward, Vector3 & up, Vector3 & right);
	};
	inline Versor operator*(Versor const & lhs, Versor const & rhs);

#include "Versor.inl"
}
