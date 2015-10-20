#pragma once

#include <cmath>

namespace eae6320
{
	struct Vector3
	{
		static const Vector3 Zero, One, I, J, K;

		Vector3();
		Vector3(float x, float y, float z);
		Vector3(Vector3 const & v);

		Vector3 operator+() const;
		Vector3 operator-() const;

		float dot(Vector3 const & rhs) const;
		Vector3 cross(Vector3 const & rhs) const;

		float norm() const;
		Vector3 & normalize();
		Vector3 unit() const;

		float x, y, z;
	};

	/* writing all these made me feel quite sycophantic */

	inline bool operator==(Vector3 const & lhs, Vector3 const & rhs);
	inline bool operator!=(Vector3 const & lhs, Vector3 const & rhs);
	inline Vector3 operator+(Vector3 const & lhs, Vector3 const & rhs);
	inline Vector3 & operator+=(Vector3 & lhs, Vector3 const & rhs);
	inline Vector3 operator-(Vector3 const & lhs, Vector3 const & rhs);
	inline Vector3 & operator-=(Vector3 & lhs, Vector3 const & rhs);
	inline float operator$(Vector3 const & lhs, Vector3 const & rhs);
	inline Vector3 operator*(Vector3 const & lhs, float const rhs);
	inline Vector3 operator*(float lhs, Vector3 const & rhs);
	inline Vector3 & operator*=(Vector3 & lhs, float rhs);
	inline Vector3 operator/(Vector3 const & lhs, float rhs);
	inline Vector3 & operator/=(Vector3 & lhs, float rhs);

#include "Vector3.inl"
}
