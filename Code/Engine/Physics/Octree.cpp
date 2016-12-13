#include "stdafx.h"

#include "Terrain.h"
#include "../Math/Triangle3.h"

namespace eae6320
{
namespace Physics
{

void Terrain::Octree::populate(const Triangle3 * triangles, uint32_t num_triangles)
{
	for (uint32_t i = 0; i < num_triangles; ++i)
		insert(i, triangles[i]);

	propagate_all(triangles, num_triangles);

	optimize(triangles, num_triangles);
}

void Terrain::Octree::insert(uint32_t id, const Triangle3 & triangle)
{
	Octree *node;
	Vector3 tri2center = triangle.box.vmin + triangle.box.vmax;

	for (node = this;
		node->max_depth > 0 && node->bounds.contains(triangle.box);
		node = node->branch[(tri2center - node->bounds.vmin - node->bounds.vmax).octant()])
	{
		if (node->is_leaf())
			node->branch_out();
	}

	node->object_ids.push_back(id);
}

void Terrain::Octree::propagate_all(const Triangle3 * triangles, uint32_t num_triangles)
{
	if (is_leaf()) return;

	for (std::vector<uint32_t>::const_iterator it = object_ids.begin(); it != object_ids.end(); ++it)
		propagate(*it, triangles[*it]);

	object_ids.clear();

	for (uint8_t i = 0; i < 8; ++i)
		branch[i]->propagate_all(triangles, num_triangles);
}

void Terrain::Octree::propagate(uint32_t id, const Triangle3 & triangle)
{
	if (!triangle.box.intersects(bounds)) return;

	if (is_leaf())
		object_ids.push_back(id);
	else for (uint8_t i = 0; i < 8; ++i)
		branch[i]->propagate(id, triangle);
}

void Terrain::Octree::optimize(const Triangle3 * triangles, uint32_t num_triangles)
{
	if (object_ids.size() / (float)(MAX_DEPTH - max_depth) > FILL_DEPTH_RATIO)
	{
		branch_out();
		propagate_all(triangles, num_triangles);
	}

	if (is_leaf()) return;

	for (uint8_t i = 0; i < 8; ++i)
		branch[i]->propagate_all(triangles, num_triangles);
}

}
}