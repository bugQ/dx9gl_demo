#pragma once

#include <cmath>

namespace eae6320
{
	struct Vector2
	{
		static const Vector2 Zero, One, I, J;

		Vector2();
		Vector2(float x, float y);
		Vector2(Vector2 const & v);

		Vector2 operator+() const;
		Vector2 operator-() const;

		float dot(const Vector2 & rhs) const;

		float norm() const;
		Vector2 & normalize();
		Vector2 & clip(float magnitude);
		Vector2 unit() const;

		float x, y, z;
	};

	/* writing all these made me feel quite sycophantic */

	inline bool operator==(Vector2 const & lhs, Vector2 const & rhs);
	inline bool operator!=(Vector2 const & lhs, Vector2 const & rhs);
	inline Vector2 operator+(Vector2 const & lhs, Vector2 const & rhs);
	inline Vector2 & operator+=(Vector2 & lhs, Vector2 const & rhs);
	inline Vector2 operator-(Vector2 const & lhs, Vector2 const & rhs);
	inline Vector2 & operator-=(Vector2 & lhs, Vector2 const & rhs);
	inline Vector2 operator*(Vector2 const & lhs, float const rhs);
	inline Vector2 operator*(float lhs, Vector2 const & rhs);
	inline Vector2 & operator*=(Vector2 & lhs, float rhs);
	inline Vector2 operator/(Vector2 const & lhs, float rhs);
	inline Vector2 & operator/=(Vector2 & lhs, float rhs);

#include "Vector2.inl"
}
