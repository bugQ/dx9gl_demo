#include "stdafx.h"
#include "Terrain.h"
#include <limits>
#include <algorithm>

namespace eae6320
{
namespace Physics
{

Triangle3 * cache_triangles(const Graphics::Mesh::Data * mesh_data, Vector3 scale)
{
	Triangle3 * triangles = new Triangle3[mesh_data->num_triangles];

	for (uint32_t i = 0; i < mesh_data->num_triangles; ++i)
	{
		Graphics::Mesh::Index i_a = mesh_data->indices[i * 3];
		Graphics::Mesh::Index i_b = mesh_data->indices[i * 3 + 1];
		Graphics::Mesh::Index i_c = mesh_data->indices[i * 3 + 2];
		Graphics::Mesh::Vertex * a = &mesh_data->vertices[i_a];
		Graphics::Mesh::Vertex * b = &mesh_data->vertices[i_b];
		Graphics::Mesh::Vertex * c = &mesh_data->vertices[i_c];
		triangles[i] = Triangle3(
			Vector3(a->x, a->y, a->z).scale(scale),
			Vector3(b->x, b->y, b->z).scale(scale),
			Vector3(c->x, c->y, c->z).scale(scale));
	}

	return triangles;
}

Terrain::Terrain(Triangle3 * triangles, size_t num_triangles)
	: triangles(triangles), num_triangles(num_triangles)
{
}

Terrain * Terrain::FromBinFile(const char * collision_mesh_path, Vector3 scale)
{
	Graphics::Mesh::Data * mesh_data = Graphics::Mesh::Data::FromBinFile(collision_mesh_path);
	Triangle3 * triangles = cache_triangles(mesh_data, scale);
	size_t num_triangles = mesh_data->num_triangles;

	delete mesh_data;
	return new Terrain(triangles, num_triangles);
}


Terrain::~Terrain()
{
	delete[] triangles;
}

bool Terrain::intersect_segment(Vector3 p, Vector3 q, float &t, Vector3 &n) const
{
	t = std::numeric_limits<float>::infinity();

	for (size_t i = 0; i < num_triangles; ++i)
	{
		float t_i;
		Vector3 n_i;

		if (triangles[i].intersect_segment(p, q, t_i, n_i) && t_i < t)
		{
			t = t_i;
			n = n_i;
		}
	}

	return std::isfinite(t);
}

}
}