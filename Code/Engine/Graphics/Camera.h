#pragma once

#include "../Math/Vector3.h"
#include "../Math/Versor.h"

namespace eae6320
{
struct Camera
{
	Vector3 position;
	Versor rotation;
};
}
