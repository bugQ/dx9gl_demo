#pragma once

#include "Vector3.h"
#include "Vector4.h"
#include "Versor.h"

namespace eae6320
{
	struct Matrix4
	{
		static const Matrix4 Zero, Identity;

		// WARNING!  no initialization!
		Matrix4();

		Matrix4(Matrix4 const &);

		// mostly for internal use
		Matrix4(
			float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33);

		// scale matrix creator
		static Matrix4 scale(Vector3 const & s);
		static Matrix4 scale(float sx, float sy, float sz);
		static Matrix4 diagonal(Vector4 const & v);

		// rotation matrix creators. COLUMN MAJOR
		static Matrix4 rotation_x(float radians);
		static Matrix4 rotation_y(float radians);
		static Matrix4 rotation_z(float radians);
		static Matrix4 rotation_zyx(Vector3 const & r);
		static Matrix4 rotation_xyz(Vector3 const & r);
		static Matrix4 rotation_q(Versor const & q);

		static Matrix4 create_RT(Versor const & q, Vector3 const & p);
		static Matrix4 inverse_RT(Versor const & q, Vector3 const & p);

		Matrix4 & operator=(Matrix4 const & rhs);

		// DIRECT ACCESS
		Vector4 & vec(size_t i);
		Vector3 & vec3(size_t i);

		Matrix4 dot(Matrix4 const & rhs) const;
		Vector4 predot(Vector4 const & lhs) const;
		Vector4 predot0(Vector3 const & lhs) const;
		Vector4 predot1(Vector3 const & lhs) const;
		Vector4 postdot(Vector4 const & rhs) const;
		Vector4 postdot0(Vector3 const & rhs) const;
		Vector4 postdot1(Vector3 const & rhs) const;

		float determinant();
		Matrix4 inverse();
		Matrix4 transpose();

		float m[4][4];
	};

	inline bool operator==(Matrix4 const & lhs, Matrix4 const & rhs);
	inline bool operator!=(Matrix4 const & lhs, Matrix4 const & rhs);
	inline Matrix4 operator*(Matrix4 const & lhs, Matrix4 const & rhs);
	inline Matrix4 & operator*=(Matrix4 & lhs, Matrix4 const & rhs);
	inline Vector4 operator*(Matrix4 const & lhs, Vector4 const & rhs);
	inline Vector4 operator*(Vector4 const & lhs, Matrix4 const & rhs);
	inline Matrix4 operator*(Matrix4 const & lhs, float rhs);
	inline Matrix4 operator*(float lhs, Matrix4 const & rhs);
	inline Matrix4 & operator*=(Matrix4 & lhs, float rhs);

#include "Matrix4.inl"
}
