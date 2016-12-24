inline Versor::Versor() : Vector4(0, 0, 0, 1) {}
inline Versor::Versor(float x, float y, float z, float w) : Vector4(x, y, z, w) {}
inline Versor::Versor(Vector4 const & v) : Vector4(v) {}

inline Versor operator*(Versor const & lhs, Versor const & rhs)
{
	return Versor
		( lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y
		, lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z
		, lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x
		, lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z
		);
}

inline Versor Versor::inverse() const
{
	return Versor(-x, -y, -z, w);
}

inline Versor Versor::rotate_by(Versor rotation) const
{
	return rotation * (*this) * rotation.inverse();
}

inline Vector3 Versor::rotate(Vector3 direction) const
{
	return Versor(Vector4(direction, 0.0f)).rotate_by(*this).xyz();
}

inline Versor Versor::rotation_x(float radians)
{
	float half = radians * 0.5f;
	float c = cos(half), s = sin(half);
	return Versor(s, 0.0f, 0.0f, c);
}

inline Versor Versor::rotation_y(float radians)
{
	float half = radians * 0.5f;
	float c = cos(half), s = sin(half);
	return Versor(0.0f, s, 0.0f, c);

}

inline Versor Versor::rotation_z(float radians)
{
	float half = radians * 0.5f;
	float c = cos(half), s = sin(half);
	return Versor(0.0f, 0.0f, s, c);
}

inline Versor Versor::orientation(const Vector3 & forward, Vector3 & up, Vector3 & right)
{
	up = forward.orthonormal(up);
	right = up.cross(forward);

	Versor q(up.z - forward.y, forward.x - right.z, right.y - up.x, right.x + up.y + forward.z + 1.0f);
	return q.normalize();
}