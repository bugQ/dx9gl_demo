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
	Vector3 velocity;

	Collider(Vector3 position, float yaw, float height)
		: UprightEntity(position, yaw), height(height), velocity(0,0,0) {}
	virtual ~Collider() {}

	// returns true if colliding with ground
	bool move(Vector3 displacement, const Terrain & terrain);
};
}
}