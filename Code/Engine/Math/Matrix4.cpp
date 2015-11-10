#include "Matrix4.h"
#include <cmath>

namespace eae6320
{
	const Matrix4 Matrix4::Zero = Matrix4(0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);
	const Matrix4 Matrix4::Identity = Matrix4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
	
// SKIP(i,n) equivalent to ( n<i ? n : n+1 )
#define SKIP(i,n) ((n)+((i)<=(n)))
// MINOR(i,j) equal to the determinant of the minor of m[i][j] for 4x4 array m
#define MINOR(i,j) \
	( m[SKIP(i,0)][SKIP(j,0)] * \
		( m[SKIP(i,1)][SKIP(j,1)] * m[SKIP(i,2)][SKIP(j,2)] \
		- m[SKIP(i,2)][SKIP(j,1)] * m[SKIP(i,1)][SKIP(j,2)] ) \
	- m[SKIP(i,0)][SKIP(j,1)] * \
		( m[SKIP(i,1)][SKIP(j,0)] * m[SKIP(i,2)][SKIP(j,2)] \
		- m[SKIP(i,2)][SKIP(j,0)] * m[SKIP(i,1)][SKIP(j,2)] ) \
	+ m[SKIP(i,0)][SKIP(j,2)] * \
		( m[SKIP(i,1)][SKIP(j,0)] * m[SKIP(i,2)][SKIP(j,1)] \
		- m[SKIP(i,2)][SKIP(j,0)] * m[SKIP(i,1)][SKIP(j,1)] ) )

	float Matrix4::determinant()
	{
		float min00 = MINOR(0,0), min01 = MINOR(0,1), min02 = MINOR(0,2), min03 = MINOR(0,3);

		return m[0][0]*min00 - m[0][1]*min01 + m[0][2]*min02 - m[0][3]*min03;
	}

	Matrix4 Matrix4::inverse()
	{
		Matrix4 adjoint(
			+MINOR(0,0), -MINOR(1,0), +MINOR(2,0), -MINOR(3,0),
			-MINOR(0,1), +MINOR(1,1), -MINOR(2,1), +MINOR(3,1),
			+MINOR(0,2), -MINOR(1,2), +MINOR(2,2), -MINOR(3,2),
			-MINOR(0,3), +MINOR(1,3), -MINOR(2,3), +MINOR(3,3)
		);

		float determinant =
			  m[0][0] * adjoint.m[0][0] + m[0][1] * adjoint.m[1][0]
			+ m[0][2] * adjoint.m[2][0] + m[0][3] * adjoint.m[3][0];

		return (1.0f / determinant) * adjoint;
	}
	
	Matrix4 Matrix4::rotation_zyx(Vector3 const & r)
	{
		return rotation_z(r.z) * rotation_y(r.y) * rotation_x(r.x);
	}

	Matrix4 Matrix4::rotation_xyz(Vector3 const & r)
	{
		return rotation_x(r.x) * rotation_y(r.y) * rotation_z(r.z);
	}
}