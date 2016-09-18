#pragma once

#include "UprightEntity.h"
#include "Terrain.h"

namespace eae6320
{
namespace Physics
{
struct Collider : public UprightEntity
{
	float height;

	Collider(Vector3 position, float yaw, float height)
		: UprightEntity(position, yaw), height(height) {}
	virtual ~Collider() {}

	void move(Vector3 displacement, const Terrain & terrain);
};
}
}