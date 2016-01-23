#include "stdafx.h"
#include "Camera.h"

namespace eae6320
{

Camera::Camera()
{
}

Versor Camera::rotation()
{
	return Versor::rotation_y(yaw);
}

}