inline Versor::Versor() : Vector4(0, 0, 0, 0) {}
inline Versor::Versor(float x, float y, float z, float w) : Vector4(x, y, z, w) {}
inline Versor::Versor(Vector4 const & v) : Vector4(v) {}

inline Versor operator*(Versor const & lhs, Versor const & rhs)
{
	return Versor(
		lhs.w * rhs.w - lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z,
		lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z,
		lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x);
}
