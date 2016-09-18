#include "Triangle3.h"
#include <limits>

namespace eae6320
{
	/*
	float Triangle3::intersect_ray(Vector3 o, Vector3 dir) const
	{
		float t = normal.dot(a - o) / normal.dot(dir);
		Vector3 u0 = dir, v0 = dir.cross(o);
		Vector3 u1 = a - b, v1 = a.cross(b);
		Vector3 u2 = b - c, v2 = b.cross(c);
		Vector3 u3 = c - a, v3 = c.cross(a);
		float w1 = u0.dot(v1) + u1.dot(v0);
		float w2 = u0.dot(v2) + u2.dot(v0);
		float w3 = u0.dot(v3) + u3.dot(v0);

		if ((w1 < 0 && w2 < 0 && w3 < 0) || (w1 > 0 && w2 > 0 && w3 > 0))
			return t;
		else return std::numeric_limits<float>::infinity();
	}
	*/

	/*
	float Triangle3::intersect_ray(Vector3 o, Vector3 dir) const
	{
		Vector3 oa = a - o, ob = b - o, oc = c - o;
		float d = normal.dot(dir);
		if (d < 1e-9f && d > -1e-9f)
			return std::numeric_limits<float>::infinity();
		float t = normal.dot(oa) / normal.dot(dir);
		float w1 = dir.dot(oa.cross(ob));
		float w2 = dir.dot(ob.cross(oc));
		float w3 = dir.dot(oc.cross(oa));

		if ((w1 <= 0 && w2 <= 0 && w3 <= 0) || (w1 >= 0 && w2 >= 0 && w3 >= 0))
			return t;
		else return std::numeric_limits<float>::infinity();
	}
	*/

	
	float Triangle3::intersect_ray(Vector3 o, Vector3 dir) const
	{
		float diverge = std::numeric_limits<float>::infinity();
		Vector3 ab = b - a;
		Vector3 ac = c - a;
		float d = -normal.dot(dir);
		if (d < 1e-9f && d > -1e-9f)
			return diverge;
		Vector3 ao = o - a;
		float t = normal.dot(ao);
		if (t < 0 || t > d)
			return diverge;
		t /= d;

		Vector3 w = ao + dir * t;
		float uu, uv, vv, wu, wv, D;

		uu = ab.dot(ab);
		uv = ab.dot(ac);
		vv = ac.dot(ac);
		wu = w.dot(ab);
		wv = w.dot(ac);
		D = uv * uv - uu * vv;

		float si, ti;
		si = (uv * wv - vv * wu) / D;
		if (si < 0.0 || si > 1.0)         // I is outside T
			return diverge;
		ti = (uv * wu - uu * wv) / D;
		if (ti < 0.0 || (si + ti) > 1.0)  // I is outside T
			return diverge;

		return t;
	}
/*
inline int max_dim(Vector3 v)
{
	float x = fabs(v.x), y = fabs(v.y), z = fabs(v.z);
	return x > z ? (x > y ? 0 : 2) : (y > z ? 1 : 2);
}

void swap(int &a, int &b)
{
	a = a^b;
	b = a^b;
	a = a^b;
}

float fxor(const float &a, const float &b)
{
	long la = *reinterpret_cast<const long *>(&a);
	long lb = *reinterpret_cast<const long *>(&b);
	long lc = la^lc;
	return *reinterpret_cast<float *>(&lc);
}

float Triangle3::intersect_ray(Vector3 o, Vector3 dir) const
{
	float fail = std::numeric_limits<float>::infinity();
	int kz = max_dim(dir);
	int kx = kz + 1; if (kx == 3) kx = 0;
	int ky = kx + 1; if (ky == 3) ky = 0;
	if (dir[kz] < 0.0f) swap(kx, ky);
	float Sx = dir[kx] / dir[kz];
	float Sy = dir[ky] / dir[kz];
	float Sz = 1.0f / dir[kz];
	const Vector3 A = a - o;
	const Vector3 B = b - o;
	const Vector3 C = c - o;
	const float Ax = A[kx] - Sx*A[kz];
	const float Ay = A[ky] - Sy*A[kz];
	const float Bx = B[kx] - Sx*B[kz];
	const float By = B[ky] - Sy*B[kz];
	const float Cx = C[kx] - Sx*C[kz];
	const float Cy = C[ky] - Sy*C[kz];
	float U = Cx*By - Cy*Bx;
	float V = Ax*Cy - Ay*Cx;
	float W = Bx*Ay - By*Ax;
	if (U == 0.0f || V == 0.0f || W == 0.0f) {
		double CxBy = (double)Cx*(double)By;
		double CyBx = (double)Cy*(double)Bx;
		U = (float)(CxBy - CyBx);
		double AxCy = (double)Ax*(double)Cy;
		double AyCx = (double)Ay*(double)Cx;
		V = (float)(AxCy - AyCx);
		double BxAy = (double)Bx*(double)Ay;
		double ByAx = (double)By*(double)Ax;
		W = (float)(BxAy - ByAx);
	}
	if ((U<0.0f || V<0.0f || W<0.0f) &&
		(U>0.0f || V>0.0f || W>0.0f)) return fail;
	float det = U + V + W;
	if (det == 0.0f) return fail;
	const float Az = Sz*A[kz];
	const float Bz = Sz*B[kz];
	const float Cz = Sz*C[kz];
	float T = U*Az + V*Bz + W*Cz;
	T = fxor(T, det * 0.f);
	if (T < 0) return fail;
	float t = T / det;
	return t;
}
*/
}