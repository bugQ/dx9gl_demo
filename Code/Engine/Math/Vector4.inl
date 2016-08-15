inline Vector4::Vector4() {}
inline Vector4::Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
inline Vector4::Vector4(Vector4 const & v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
inline Vector4::Vector4(Vector3 const & v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}

inline Vector3 Vector4::xyz() const {
	return Vector3(x, y, z);
}

inline Vector3 Vector4::rgb() const {
	return xyz();
}

inline Vector4 Vector4::operator+() const {
	return Vector4(+x, +y, +z, +w);
}

inline Vector4 Vector4::operator-() const {
	return Vector4(-x, -y, -z, -w);
}

inline float Vector4::dot(Vector4 const & rhs) const {
	return x*rhs.x + y*rhs.y + z*rhs.z + w*rhs.w;
}

inline float Vector4::norm() const
{
	return sqrt(x*x + y*y + z*z + w*w);
}

inline Vector4 & Vector4::normalize()
{
	float n = norm();
	if (n == 0.0f)
		return *this;
	x /= n; y /= n; z /= n; w /= n;
	return *this;
}

inline Vector4 Vector4::unit() const
{
	float n = norm();
	return Vector4(x / n, y / n, z / n, w / n);
}


/* writing all these makes me feel quite sycophantic */

inline bool operator==(Vector4 const & lhs, Vector4 const & rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

inline bool operator!=(Vector4 const & lhs, Vector4 const & rhs)
{
	return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z || lhs.w != rhs.w;
}

inline Vector4 operator+(Vector4 const & lhs, Vector4 const & rhs)
{
	return Vector4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}

inline Vector4 & operator+=(Vector4 & lhs, Vector4 const & rhs)
{
	lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z; lhs.w += rhs.w; return lhs;
}

inline Vector4 operator-(Vector4 const & lhs, Vector4 const & rhs)
{
	return Vector4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}

inline Vector4 & operator-=(Vector4 & lhs, Vector4 const & rhs)
{
	lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z; lhs.w -= rhs.w; return lhs;
}

inline Vector4 operator*(Vector4 const & lhs, float const rhs)
{
	return Vector4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);
}

inline Vector4 operator*(float lhs, Vector4 const & rhs)
{
	return Vector4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
}

inline Vector4 & operator*=(Vector4 & lhs, float rhs)
{
	lhs.x *= rhs; lhs.y *= rhs; lhs.x *= rhs; lhs.w *= rhs; return lhs;
}

inline Vector4 operator/(Vector4 const & lhs, float rhs)
{
	return Vector4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs);
}

inline Vector4 & operator/=(Vector4 & lhs, float rhs)
{
	lhs.x /= rhs; lhs.y /= rhs; lhs.z /= rhs; lhs.w /= rhs; return lhs;
}