#pragma once

#include <cmath>

#include "Vector3.h"

namespace eae6320
{
	struct Vector4
	{
		static const Vector4 Zero, One, I, J, K, L;

		Vector4();
		Vector4(float x, float y, float z, float w);
		Vector4(Vector4 const & v);
		Vector4(Vector3 const & v, float w);

		Vector3 xyz() const;
		Vector3 rgb() const;

		Vector4 operator+() const;
		Vector4 operator-() const;

		float dot(Vector4 const & rhs) const;

		float norm() const;
		Vector4 & normalize();
		Vector4 unit() const;

		union { float x, r; };
		union { float y, g; };
		union { float z, b; };
		union { float w, a; };
	};

	/* writing all these made me feel quite sycophantic */

	inline bool operator==(Vector4 const & lhs, Vector4 const & rhs);
	inline bool operator!=(Vector4 const & lhs, Vector4 const & rhs);
	inline Vector4 operator+(Vector4 const & lhs, Vector4 const & rhs);
	inline Vector4 & operator+=(Vector4 & lhs, Vector4 const & rhs);
	inline Vector4 operator-(Vector4 const & lhs, Vector4 const & rhs);
	inline Vector4 & operator-=(Vector4 & lhs, Vector4 const & rhs);
	inline Vector4 operator*(Vector4 const & lhs, float const rhs);
	inline Vector4 operator*(float lhs, Vector4 const & rhs);
	inline Vector4 & operator*=(Vector4 & lhs, float rhs);
	inline Vector4 operator/(Vector4 const & lhs, float rhs);
	inline Vector4 & operator/=(Vector4 & lhs, float rhs);

#include "Vector4.inl"
}
