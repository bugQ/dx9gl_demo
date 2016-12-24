#pragma once

#include <cmath>
#include <limits>
#include <cstdint>

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
		float operator[](size_t i) const;

		float dot(Vector3 const & rhs) const;
		Vector3 cross(Vector3 const & rhs) const;
		Vector3 scale(Vector3 const & rhs) const;
		Vector3 project(Vector3 normal) const;

		float norm() const;
		float norm_sq() const;
		Vector3 & normalize();
		Vector3 & clip(float magnitude);
		Vector3 orthonormal(Vector3 tangent) const;
		Vector3 unit() const;
		Vector3 abs() const;

		uint8_t octant() const;

		static Vector3 min3(Vector3 const & u, Vector3 const & v);
		static Vector3 max3(Vector3 const & u, Vector3 const & v);
		float max_dim() const;

		union { float x, r; };
		union { float y, g; };
		union { float z, b; };
	};

	/* writing all these made me feel quite sycophantic */

	inline bool operator==(Vector3 const & lhs, Vector3 const & rhs);
	inline bool operator!=(Vector3 const & lhs, Vector3 const & rhs);
	inline Vector3 operator+(Vector3 const & lhs, Vector3 const & rhs);
	inline Vector3 & operator+=(Vector3 & lhs, Vector3 const & rhs);
	inline Vector3 operator-(Vector3 const & lhs, Vector3 const & rhs);
	inline Vector3 & operator-=(Vector3 & lhs, Vector3 const & rhs);
	inline Vector3 operator*(Vector3 const & lhs, float const rhs);
	inline Vector3 operator*(float lhs, Vector3 const & rhs);
	inline Vector3 & operator*=(Vector3 & lhs, float rhs);
	inline Vector3 operator/(Vector3 const & lhs, float rhs);
	inline Vector3 & operator/=(Vector3 & lhs, float rhs);

#include "Vector3.inl"
}
