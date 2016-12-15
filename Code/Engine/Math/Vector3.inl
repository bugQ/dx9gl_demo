inline Vector3::Vector3() {}
inline Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
inline Vector3::Vector3(Vector3 const & v) : x(v.x), y(v.y), z(v.z) {}

inline Vector3 Vector3::operator+() const {
	return Vector3(+x, +y, +z);
}

inline Vector3 Vector3::operator-() const {
	return Vector3(-x, -y, -z);
}

inline float Vector3::operator[](size_t i) const {
	switch (i) {
	case 0: return x;
	case 1: return y;
	case 2: return z;
	default: return std::numeric_limits<float>::signaling_NaN();
	}
}

inline float Vector3::dot(Vector3 const & rhs) const {
	return x*rhs.x + y*rhs.y + z*rhs.z;
}

inline Vector3 Vector3::cross(Vector3 const & rhs) const {
	return Vector3(y*rhs.z - z*rhs.y, x*rhs.z - z*rhs.x, x*rhs.y - y*rhs.x);
}

inline Vector3 Vector3::scale(Vector3 const & rhs) const {
	return Vector3(x * rhs.x, y * rhs.y, z * rhs.z);
}

inline float Vector3::norm() const
{
	return sqrt(x*x + y*y + z*z);
}

inline Vector3 & Vector3::normalize()
{
	float n = norm();
	if (n == 0.0f)
		return *this;
	x /= n; y /= n; z /= n;
	return *this;
}

inline Vector3 Vector3::unit() const
{
	float n = norm();
	if (n == 0.0f)
		return Vector3::Zero;
	return Vector3(x / n, y / n, z / n);
}

inline uint8_t Vector3::octant() const
{
	uint8_t octant = 0;
	if (x < 0) octant += 1;
	if (y < 0) octant += 2;
	if (z < 0) octant += 4;
	return octant;
}

inline Vector3 Vector3::min3(Vector3 const & u, Vector3 const & v)
{
	return Vector3(fminf(u.x, v.x), fminf(u.y, v.y), fminf(u.z, v.z));
}

inline Vector3 Vector3::max3(Vector3 const & u, Vector3 const & v)
{
	return Vector3(fmaxf(u.x, v.x), fmaxf(u.y, v.y), fmaxf(u.z, v.z));
}

inline float Vector3::max_dim() const
{
	return fmaxf(fmaxf(x, y), z);
}

/* writing all these makes me feel quite sycophantic */

inline bool operator==(Vector3 const & lhs, Vector3 const & rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline bool operator!=(Vector3 const & lhs, Vector3 const & rhs)
{
	return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z;
}

inline Vector3 operator+(Vector3 const & lhs, Vector3 const & rhs)
{
	return Vector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

inline Vector3 & operator+=(Vector3 & lhs, Vector3 const & rhs)
{
	lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z; return lhs;
}

inline Vector3 operator-(Vector3 const & lhs, Vector3 const & rhs)
{
	return Vector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

inline Vector3 & operator-=(Vector3 & lhs, Vector3 const & rhs)
{
	lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z; return lhs;
}

inline Vector3 operator*(Vector3 const & lhs, float const rhs)
{
	return Vector3(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
}

inline Vector3 operator*(float lhs, Vector3 const & rhs)
{
	return Vector3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

inline Vector3 & operator*=(Vector3 & lhs, float rhs)
{
	lhs.x *= rhs; lhs.y *= rhs; lhs.x *= rhs; return lhs;
}

inline Vector3 operator/(Vector3 const & lhs, float rhs)
{
	return Vector3(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
}

inline Vector3 & operator/=(Vector3 & lhs, float rhs)
{
	lhs.x /= rhs; lhs.y /= rhs; lhs.z /= rhs; return lhs;
}