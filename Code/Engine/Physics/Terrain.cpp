#include "stdafx.h"
#include "Terrain.h"
#include <limits>
#include <algorithm>
#include <cassert>

namespace eae6320
{
namespace Physics
{

const float Terrain::Octree::FILL_DEPTH_RATIO = 1.5f;

Triangle3 * cache_triangles(const Graphics::Mesh::Data & mesh_data, Vector3 scale)
{
	Triangle3 * triangles = new Triangle3[mesh_data.num_triangles];

	for (uint32_t i = 0; i < mesh_data.num_triangles; ++i)
	{
		Graphics::Mesh::Index i_a = mesh_data.indices[i * 3];
		Graphics::Mesh::Index i_b = mesh_data.indices[i * 3 + 1];
		Graphics::Mesh::Index i_c = mesh_data.indices[i * 3 + 2];
		Graphics::Mesh::Vertex * a = &mesh_data.vertices[i_a];
		Graphics::Mesh::Vertex * b = &mesh_data.vertices[i_b];
		Graphics::Mesh::Vertex * c = &mesh_data.vertices[i_c];
		triangles[i] = Triangle3(a->position, b->position, c->position, b->normal).scale(scale);
	}

	return triangles;
}

Terrain::Terrain(const Graphics::Mesh::Data & mesh_data, Vector3 scale, Graphics::Wireframe & wireframe)
	: triangles(cache_triangles(mesh_data, scale))
	, num_triangles(mesh_data.num_triangles)
	, octree(mesh_data.bounds.scale(scale).square())
	, wireframe(wireframe)
{
}

Terrain * Terrain::FromBinFile(const char * collision_mesh_path, Vector3 scale, Graphics::Wireframe & wireframe)
{
	Graphics::Mesh::Data * mesh_data = Graphics::Mesh::Data::FromBinFile(collision_mesh_path);
	Terrain * terrain = new Terrain(*mesh_data, scale, wireframe);
	delete mesh_data;
	terrain->init_octree();

	return terrain;
}


float Terrain::intersect_ray(Vector3 o, Vector3 dir, Vector3 * n) const
{
	float t = std::numeric_limits<float>::infinity();
	size_t hit_i;

	/**
	std::queue<const Octree *> hitboxes;
	const Octree * node;

	octree.intersect(Segment3(o, o + dir), hitboxes);

	while (!hitboxes.empty())
	{
		node = hitboxes.front();
		hitboxes.pop();

		for (uint32_t id : node->object_ids)
		{
			Vector3 n_i = triangles[id].normal;
			float t_i = triangles[id].intersect_ray(o, dir);
			if (t_i > 0 && t_i < t)
			{
				t = t_i;
				if (n) *n = triangles[id].normal;
			}
		}
	}

	/*/ // naive impl, check entire triangle list
	for (size_t i = 0; i < num_triangles; ++i)
	{
		Vector3 n_i = triangles[i].normal;
		float t_i = triangles[i].intersect_ray(o, dir);
		if (t_i > 0 && t_i < t)
		{
			t = t_i;
			if (n) *n = triangles[i].normal;
			hit_i = i;
		}
	}
	/**/

	if (t < std::numeric_limits<float>::infinity())
	{
		wireframe.addTriangle(triangles[hit_i], Graphics::Color::White);
	}

	return t;
}

#ifdef _DEBUG
void Terrain::draw_raycast(Segment3 segment, Graphics::Wireframe & wireframe)
{
	wireframe.addLine(segment, Graphics::Color::White);

	std::queue<const Octree *> hitboxes;
	const Octree * node;

	octree.intersect(segment, hitboxes);

	while (!hitboxes.empty())
	{
		node = hitboxes.front();
		hitboxes.pop();

		node->draw(wireframe);

		for (uint32_t id : node->object_ids)
			wireframe.addTriangle(triangles[id], Graphics::Color::White);
	}

	Segment3 drop(Vector3(0,0,0),Vector3(0,-30,0));
	if (segment != drop)
		draw_raycast(drop, wireframe);
}

void Terrain::test_octree()
{
	std::vector<bool> triangle_inventory(num_triangles, false);

	octree.take_inventory(triangle_inventory);

	for (size_t i = 0; i < num_triangles; ++i)
		assert(triangle_inventory[i]);

	std::queue<const Octree *> boxes_with_4;
	octree.find(4, boxes_with_4);

	std::queue<const Octree *> hitboxes;
	octree.intersect(Segment3(Vector3(0, 0, 0), Vector3(0, -10, 0)), hitboxes);

	
}
#endif

}
}