#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Color.h"
#include "../Math/Vector3.h"
#include "../Math/Triangle3.h"
#include "../Math/Segment3.h"
#include "../Math/AABB3.h"
#include <vector>

namespace eae6320
{
namespace Graphics
{

#ifdef _DEBUG

struct Wireframe
{
	static const size_t MAXLINES = 32768;

	Mesh::Vertex points[MAXLINES * 2];
	size_t num_lines;
	Mesh * mesh;
	Material * material;

	void addLine(Vector3 p1, Color color1, Vector3 p2, Color color2);
	void addLine(Segment3 segment, Color color);
	void addTriangle(Triangle3 tri, Color color);
	void addAABB(Vector3 p0, Vector3 p7, Color color);
	void addAABB(AABB3 box, Color color);
	void addSphere(Vector3 center, float radius, uint8_t resolution, Color color);
	void addCylinder(Vector3 center, float radius, float extent, uint8_t resolution, Color color);

	void clear();

	Wireframe(Material * material);
	~Wireframe();
};

#else

struct Wireframe
{
	void addLine(Vector3 p1, Color color1, Vector3 p2, Color color2) {}
	void addAABB(Vector3 center, Vector3 extents, Color color) {}
	void addAABB(AABB3 box, Color color) {}
	void addSphere(Vector3 center, float radius, uint8_t resolution, Color color) {}
	void addSphere(Vector3 center, float radius, uint8_t resolution, Color color) {}
	void addCylinder(Vector3 center, float radius, float extent, uint8_t resolution, Color color) {}

	void clear() {}

	Wireframe(Material * material) {}
	~Wireframe() {}
};

#endif

}
}

