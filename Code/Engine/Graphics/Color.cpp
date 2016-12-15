#include "stdafx.h"
#include "Color.h"


namespace eae6320
{
namespace Graphics
{

Color Color::fromHSV(float hue, float sat, float val)
{

	float hh, p, q, t, ff;
	int i;
	Color out;

	if (sat <= 0.0) {       // < is bogus, just shuts up warnings
		out.r = val;
		out.g = val;
		out.b = val;
		return out;
	}
	hh = hue;
	if (hh >= 360.0f) hh = 0.0f;
	hh /= 60.0f;
	i = (int)hh;
	ff = hh - i;
	p = val * (1.0f - sat);
	q = val * (1.0f - (sat * ff));
	t = val * (1.0f - (sat * (1.0f - ff)));

	switch (i) {
	case 0:
		out.r = val;
		out.g = t;
		out.b = p;
		break;
	case 1:
		out.r = q;
		out.g = val;
		out.b = p;
		break;
	case 2:
		out.r = p;
		out.g = val;
		out.b = t;
		break;

	case 3:
		out.r = p;
		out.g = q;
		out.b = val;
		break;
	case 4:
		out.r = t;
		out.g = p;
		out.b = val;
		break;
	case 5:
	default:
		out.r = val;
		out.g = p;
		out.b = q;
		break;
	}
	return out;
}

}
}