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
			Vector3(c->x, c->y, c->z).scale(scale),
			Vector3(b->nx, b->ny, b->nz)
		);
	}

	return triangles;
}

Terrain::Terrain(Triangle3 * triangles, size_t num_triangles)
	: triangles(triangles), num_triangles(num_triangles)
{
}

Terrain * Terrain::FromBinFile(const char * collision_mesh_path, Vector3 scale)
{
	/**/
	Graphics::Mesh::Data * mesh_data = Graphics::Mesh::Data::FromBinFile(collision_mesh_path);
	Triangle3 * triangles = cache_triangles(mesh_data, scale);
	size_t num_triangles = mesh_data->num_triangles;

	delete mesh_data;
	/*/
	size_t num_triangles = 3;
	Triangle3 * triangles = new Triangle3[num_triangles]
	{
		Triangle3(Vector3(-1, -1, 0), Vector3(-1, 1, 0), Vector3(1, -1, 0)),
		Triangle3(Vector3(0, 1, 2), Vector3(2, 1, 3), Vector3(1, -1, 3)),
		Triangle3(Vector3(2, 1, 2), Vector3(2 ,1, 3), Vector3(0, 1, 2))
	};
	/**/
	return new Terrain(triangles, num_triangles);
}


Terrain::~Terrain()
{
	delete[] triangles;
}

float Terrain::intersect_ray(Vector3 o, Vector3 dir, Vector3 * n) const
{
	float t = std::numeric_limits<float>::infinity();
	for (size_t i = 0; i < num_triangles; ++i)
	{
		Vector3 n_i = triangles[i].normal;
		float t_i = triangles[i].intersect_ray(o, dir);
		if (t_i > 0 && t_i < t)
		{
			t = t_i;
			if (n) *n = triangles[i].normal;
		}
	}
	return t;
}

}
}