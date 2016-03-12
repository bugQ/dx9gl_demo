#pragma once

#include "../Graphics/Mesh.h"

namespace eae6320
{
namespace Physics
{
class Terrain
{
	const Graphics::Mesh::Data * collision_mesh;
public:
	Terrain(const char * collision_mesh_path);
	~Terrain();

	bool SegmentCollides();
};
}
}