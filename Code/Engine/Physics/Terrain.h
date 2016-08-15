#pragma once

#include "../Graphics/Mesh.h"
#include "../Math/Triangle3.h"

namespace eae6320
{
namespace Physics
{
	struct Terrain
	{
		const Triangle3 * triangles;
		const size_t num_triangles;
	
		Terrain(Triangle3 *, size_t);
	
		static Terrain * FromBinFile(const char * collision_mesh_path, Vector3 scale);
		~Terrain();

		bool intersect_segment(Vector3 p, Vector3 q, float &t, Vector3 &n) const;
	};
}
}