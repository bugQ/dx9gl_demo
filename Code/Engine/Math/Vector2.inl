inline Vector2::Vector2() {}
inline Vector2::Vector2(float x, float y) : x(x), y(y) {}
inline Vector2::Vector2(Vector2 const & v) : x(v.x), y(v.y) {}

inline Vector2 Vector2::operator+() const {
	return Vector2(+x, +y);
}

inline Vector2 Vector2::operator-() const {
	return Vector2(-x, -y);
}

inline float Vector2::dot(const Vector2 & rhs) const {
	return x*rhs.x + y*rhs.y;
}


inline float Vector2::norm() const
{
	return sqrt(x*x + y*y);
}

inline Vector2 & Vector2::normalize()
{
	float n = norm();
	if (n == 0.0f)
		return *this;
	return *this /= n;
}

inline Vector2 & Vector2::clip(float magnitude)
{
	float n = norm(), m = fabsf(magnitude);
	if (n < m)
		return *this;
	return *this *= m / n;
}

inline Vector2 Vector2::unit() const
{
	float n = norm();
	if (n == 0.0f)
		return Vector2::Zero;
	return Vector2(x / n, y / n);
}


/* writing all these makes me feel quite sycophantic */

inline bool operator==(Vector2 const & lhs, Vector2 const & rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator!=(Vector2 const & lhs, Vector2 const & rhs)
{
	return lhs.x != rhs.x || lhs.y != rhs.y;
}

inline Vector2 operator+(Vector2 const & lhs, Vector2 const & rhs)
{
	return Vector2(lhs.x + rhs.x, lhs.y + rhs.y);
}

inline Vector2 & operator+=(Vector2 & lhs, Vector2 const & rhs)
{
	lhs.x += rhs.x; lhs.y += rhs.y; return lhs;
}

inline Vector2 operator-(Vector2 const & lhs, Vector2 const & rhs)
{
	return Vector2(lhs.x - rhs.x, lhs.y - rhs.y);
}

inline Vector2 & operator-=(Vector2 & lhs, Vector2 const & rhs)
{
	lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs;
}

inline Vector2 operator*(Vector2 const & lhs, float const rhs)
{
	return Vector2(lhs.x * rhs, lhs.y * rhs);
}

inline Vector2 operator*(float lhs, Vector2 const & rhs)
{
	return Vector2(lhs * rhs.x, lhs * rhs.y);
}

inline Vector2 & operator*=(Vector2 & lhs, float rhs)
{
	lhs.x *= rhs; lhs.y *= rhs; return lhs;
}

inline Vector2 operator/(Vector2 const & lhs, float rhs)
{
	return Vector2(lhs.x / rhs, lhs.y / rhs);
}

inline Vector2 & operator/=(Vector2 & lhs, float rhs)
{
	lhs.x /= rhs; lhs.y /= rhs; return lhs;
}