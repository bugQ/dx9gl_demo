inline Matrix4::Matrix4()
{
	for (size_t i = 0; i < 4; ++i)
		for (size_t j = 0; j < 4; ++j)
			m[i][j] = 0.0f;
}

inline Matrix4 & Matrix4::operator=(Matrix4 const & rhs)
{
	for (size_t i = 0; i < 4; ++i)
		for (size_t j = 0; j < 4; ++j)
			m[i][j] = rhs.m[i][j];

	return *this;
}

inline Matrix4::Matrix4(Matrix4 const & other)
{
	this->operator=(other);
}

inline Matrix4::Matrix4(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
	m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
	m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
	m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
}

inline Matrix4 Matrix4::scale(Vector3 const & s)
{
	return Matrix4(s.x,0,0,0, 0,s.y,0,0, 0,0,s.z,0, 0,0,0,1);
}

inline Matrix4 Matrix4::diagonal(Vector4 const & v)
{
	return Matrix4(v.x,0,0,0, 0,v.y,0,0, 0,0,v.z,0, 0,0,0,v.w);
}

inline Matrix4 Matrix4::scale(float x, float y, float z)
{
	return Matrix4(x,0,0,0, 0,y,0,0, 0,0,z,0, 0,0,0,1);
}

inline Matrix4 Matrix4::rotation_x(float radians)
{
	float s = sin(radians), c = cos(radians), n = -s;
	return Matrix4(1,0,0,0, 0,c,s,0, 0,n,c,0, 0,0,0,1);
}

inline Matrix4 Matrix4::rotation_y(float radians)
{
	float s = sin(radians), c = cos(radians), n = -s;
	return Matrix4(c,0,n,0, 0,1,0,0, s,0,c,0, 0,0,0,1);
}

inline Matrix4 Matrix4::rotation_z(float radians)
{
	float s = sin(radians), c = cos(radians), n = -s;
	return Matrix4(c,s,0,0, n,c,0,0, 0,0,1,0, 0,0,0,1);
}

inline Matrix4 Matrix4::rotation_q(Versor const & q)
{
	return Matrix4(
		q.w, q.z, -q.y, q.x,
		-q.z, q.w, q.x, q.y,
		q.y, -q.x, q.w, q.z,
		-q.x, -q.y, -q.z, q.w).dot(Matrix4(
			q.w, q.z, -q.y, -q.x,
			-q.z, q.w, q.x, -q.y,
			q.y, -q.x, q.w, -q.z,
			q.x, q.y, q.z, q.w));
}

inline Matrix4 Matrix4::create_RT(Versor const & q, Vector3 const & p)
{
	Matrix4 rt = rotation_q(q);
	rt.vec3(3) = p;
	return rt;
}

inline Matrix4 Matrix4::inverse_RT(Versor const & q, Vector3 const & p)
{
	Matrix4 irt = rotation_q(q).transpose();
	irt.vec(3) = irt.predot1(p);
	return irt;
}

inline Matrix4 Matrix4::transpose()
{
	return Matrix4(
		m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]);
}

inline Vector4 & Matrix4::vec(size_t i)
{
	return reinterpret_cast<Vector4 &>(m[i]);
}

inline Vector3 & Matrix4::vec3(size_t i)
{
	return reinterpret_cast<Vector3 &>(m[i]);
}

inline Matrix4 Matrix4::dot(Matrix4 const & rhs) const
{
	Matrix4 result;

	for (size_t i = 0; i < 4; ++i)
		for (size_t j = 0; j < 4; ++j)
			for (size_t k = 0; k < 4; ++k)
				result.m[i][j] += m[i][k] * rhs.m[k][j];

	return result;
}

inline Vector4 Matrix4::predot(Vector4 const & lhs) const
{
	return Vector4(
		lhs.x*m[0][0] + lhs.y*m[1][0] + lhs.z*m[2][0] + lhs.w*m[3][0],
		lhs.x*m[0][1] + lhs.y*m[1][1] + lhs.z*m[2][1] + lhs.w*m[3][1],
		lhs.x*m[0][2] + lhs.y*m[1][2] + lhs.z*m[2][2] + lhs.w*m[3][2],
		lhs.x*m[0][3] + lhs.y*m[1][3] + lhs.z*m[2][3] + lhs.w*m[3][3]);
}

inline Vector4 Matrix4::predot0(Vector3 const & lhs) const
{
	return Vector4(
		lhs.x*m[0][0] + lhs.y*m[1][0] + lhs.z*m[2][0],
		lhs.x*m[0][1] + lhs.y*m[1][1] + lhs.z*m[2][1],
		lhs.x*m[0][2] + lhs.y*m[1][2] + lhs.z*m[2][2],
		0);
}

inline Vector4 Matrix4::predot1(Vector3 const & lhs) const
{
	return Vector4(
		lhs.x*m[0][0] + lhs.y*m[1][0] + lhs.z*m[2][0] + m[3][0],
		lhs.x*m[0][1] + lhs.y*m[1][1] + lhs.z*m[2][1] + m[3][1],
		lhs.x*m[0][2] + lhs.y*m[1][2] + lhs.z*m[2][2] + m[3][2],
		1);
}

inline Vector4 Matrix4::postdot(Vector4 const & rhs) const
{
	return Vector4(
		m[0][0]*rhs.x + m[0][1]*rhs.y + m[0][2]*rhs.z + m[0][3]*rhs.w,
		m[1][0]*rhs.x + m[1][1]*rhs.y + m[1][2]*rhs.z + m[1][3]*rhs.w,
		m[2][0]*rhs.x + m[2][1]*rhs.y + m[2][2]*rhs.z + m[2][3]*rhs.w,
		m[3][0]*rhs.x + m[3][1]*rhs.y + m[3][2]*rhs.z + m[3][3]*rhs.w);
}

inline Vector4 Matrix4::postdot0(Vector3 const & rhs) const
{
	return Vector4(
		m[0][0]*rhs.x + m[0][1]*rhs.y + m[0][2]*rhs.z,
		m[1][0]*rhs.x + m[1][1]*rhs.y + m[1][2]*rhs.z,
		m[2][0]*rhs.x + m[2][1]*rhs.y + m[2][2]*rhs.z,
		0);
}
inline Vector4 Matrix4::postdot1(Vector3 const & rhs) const
{
	return Vector4(
		m[0][0]*rhs.x + m[0][1]*rhs.y + m[0][2]*rhs.z + m[0][3],
		m[1][0]*rhs.x + m[1][1]*rhs.y + m[1][2]*rhs.z + m[1][3],
		m[2][0]*rhs.x + m[2][1]*rhs.y + m[2][2]*rhs.z + m[2][3],
		1);
}

inline bool operator==(Matrix4 const & lhs, Matrix4 const & rhs)
{
	for (size_t i = 0; i < 4; ++i)
		for (size_t j = 0; j < 4; ++j)
			if (lhs.m[i][j] != rhs.m[i][j])
				return false;
	return true;
}

inline bool operator!=(Matrix4 const & lhs, Matrix4 const & rhs)
{
	for (size_t i = 0; i < 4; ++i)
		for (size_t j = 0; j < 4; ++j)
			if (lhs.m[i][j] == rhs.m[i][j])
				return true;
	return false;
}

inline Matrix4 operator*(Matrix4 const & lhs, Matrix4 const & rhs)
{
	return lhs.dot(rhs);
}

inline Matrix4 & operator*=(Matrix4 & lhs, Matrix4 const & rhs)
{
	// can't really multiply in place, can I?
	return lhs = lhs.dot(rhs);
}

inline Vector4 operator*(Matrix4 const & lhs, Vector4 const & rhs)
{
	return lhs.postdot(rhs);
}

inline Vector4 operator*(Vector4 const & lhs, Matrix4 const & rhs)
{
	return rhs.predot(lhs);
}

inline Matrix4 operator*(Matrix4 const & lhs, float rhs)
{
	return Matrix4(
		lhs.m[0][0] * rhs, lhs.m[0][1] * rhs, lhs.m[0][2] * rhs, lhs.m[0][3] * rhs,
		lhs.m[1][0] * rhs, lhs.m[1][1] * rhs, lhs.m[1][2] * rhs, lhs.m[1][3] * rhs,
		lhs.m[2][0] * rhs, lhs.m[2][1] * rhs, lhs.m[2][2] * rhs, lhs.m[2][3] * rhs,
		lhs.m[3][0] * rhs, lhs.m[3][1] * rhs, lhs.m[3][2] * rhs, lhs.m[3][3] * rhs);
}

inline Matrix4 operator*(float lhs, Matrix4 const & rhs)
{
	return rhs * lhs;
}

inline Matrix4 & operator*=(Matrix4 & lhs, float rhs)
{
	for (size_t i = 0; i < 4; ++i)
		for (size_t j = 0; j < 4; ++j)
			lhs.m[i][j] *= rhs;
	return lhs;
}
