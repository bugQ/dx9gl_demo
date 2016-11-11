#include "Triangle3.h"
#include <limits>
#include <assert.h>

namespace eae6320
{
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
}