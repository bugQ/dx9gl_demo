#include "stdafx.h"

#include "Terrain.h"

#include <queue>


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

	for ( node = this
		; node->max_depth > 0 && node->bounds.contains(triangle.box)
		; node = node->branch[(tri2center - node->bounds.vmin - node->bounds.vmax).octant()])
	{
		if (node->is_leaf())
			node->branch_out();
	}

	node->object_ids.push_back(id);
}

void Terrain::Octree::propagate_all(const Triangle3 * triangles, uint32_t num_triangles)
{
	if (is_leaf()) return;

	for ( std::vector<uint32_t>::const_iterator it = object_ids.begin()
		; it != object_ids.end()
		; ++it )
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
	else
		for (uint8_t i = 0; i < 8; ++i)
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

size_t Terrain::Octree::intersect(Segment3 segment, std::queue<const Octree *> & hitboxes) const
{
	if (!bounds.intersects(segment))
		return 0;

	if (is_leaf())
	{
		hitboxes.push(this);
		return 1;
	}

	size_t count = 0;
	for (uint8_t i = 0; i < 8; ++i)
		count += branch[i]->intersect(segment, hitboxes);
	return count;
}

#ifdef _DEBUG
void Terrain::Octree::draw(Graphics::Wireframe & wireframe) const
{
	std::queue<const Octree *> queue;
	const Octree * node;
	queue.push(this);
	
	while (!queue.empty())
	{
		node = queue.front();
		queue.pop();

		if (node->is_leaf())
		{
			float hue = (MAX_DEPTH - node->max_depth) * 360.0f / MAX_DEPTH;
			Graphics::Color depth_color = Graphics::Color::fromHSV(hue, 1.0f, 0.5f);
			wireframe.addAABB(node->bounds, depth_color);
		}
		else
		{
			for (uint8_t i = 0; i < 8; ++i)
				queue.push(node->branch[i]);
		}
	}
}
#endif

}
}