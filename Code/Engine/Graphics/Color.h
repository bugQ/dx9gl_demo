#pragma once

#include "../Math/Vector4.h"

namespace eae6320
{
namespace Graphics
{

struct Color : public Vector4
{
	Color() {}
	Color(float r, float g, float b, float a) : Vector4(r,g,b,a) { }
	
	static Color fromHSV(float hue, float sat, float val);
};

}
}

