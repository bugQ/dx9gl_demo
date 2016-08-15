#pragma once
#include "Mesh.h"
#include "Material.h"
#include "../Math/Vector3.h"
#include "../Math/Vector4.h"
#include <vector>

namespace eae6320
{
namespace Graphics
{

#ifdef _DEBUG

struct Wireframe
{
	static const size_t MAXLINES = 5000;

	Mesh::Vertex points[MAXLINES * 2];
	size_t num_lines;
	Mesh * mesh;
	Material * material;

	void addLine(Vector3 p1, Vector4 color1, Vector3 p2, Vector4 color2);
	void addAABB(Vector3 center, Vector3 extents, Vector4 color);
	void addSphere(Vector3 center, float radius, uint8_t resolution, Vector4 color);
	void addCylinder(Vector3 center, float radius, float extent, uint8_t resolution, Vector4 color);

	void clear();

	Wireframe(Material * material);
	~Wireframe();
};

#else

struct Wireframe
{
	inline void addLine(Vector3 p1, Vector4 color1, Vector3 p2, Vector4 color2) {}
	inline void addAABB(Vector3 center, Vector3 extents, Vector4 color) {}
	inline void addSphere(Vector3 center, float radius, uint8_t resolution, Vector4 color) {}

	inline void clear() {}

	Wireframe(Material * material) {}
	~Wireframe() {}
};

#endif

}
}

