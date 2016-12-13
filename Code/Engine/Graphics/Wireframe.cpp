#include "stdafx.h"

#ifdef _DEBUG
#include "Wireframe.h"
#include <math.h>

namespace eae6320
{
namespace Graphics
{

Wireframe::Wireframe(Material * material)
	: mesh(new Mesh()), material(material), num_lines(0)
{
}


Wireframe::~Wireframe()
{
	delete mesh;
}

void Wireframe::addLine(Vector3 p1, Vector4 color1, Vector3 p2, Vector4 color2)
{
	if (num_lines >= MAXLINES)
		return;

	Mesh::Vertex & start = points[num_lines * 2];
	start.position = p1;
	start.r = static_cast<uint8_t>(color1.x * 255);
	start.g = static_cast<uint8_t>(color1.y * 255);
	start.b = static_cast<uint8_t>(color1.z * 255);
	start.a = static_cast<uint8_t>(color1.w * 255);

	Mesh::Vertex & end = points[num_lines * 2 + 1];
	end.position = p2;
	end.r = static_cast<uint8_t>(color2.x * 255);
	end.g = static_cast<uint8_t>(color2.y * 255);
	end.b = static_cast<uint8_t>(color2.z * 255);
	end.a = static_cast<uint8_t>(color2.w * 255);

	++num_lines;
}

void Wireframe::addAABB(Vector3 center, Vector3 extents, Vector4 color)
{
	Vector3 p0 = center - extents;
	Vector3 p7 = center + extents;
	Vector3 p1 = Vector3(p7.x, p0.y, p0.z);
	Vector3 p2 = Vector3(p0.x, p7.y, p0.z);
	Vector3 p3 = Vector3(p7.x, p7.y, p0.z);
	Vector3 p4 = Vector3(p0.x, p0.y, p7.z);
	Vector3 p5 = Vector3(p7.x, p0.y, p7.z);
	Vector3 p6 = Vector3(p0.x, p7.y, p7.z);

	addLine(p0, color, p1, color);
	addLine(p0, color, p2, color);
	addLine(p0, color, p4, color);
	addLine(p1, color, p3, color);
	addLine(p1, color, p5, color);
	addLine(p2, color, p3, color);
	addLine(p2, color, p6, color);
	addLine(p3, color, p7, color);
	addLine(p4, color, p5, color);
	addLine(p4, color, p6, color);
	addLine(p5, color, p7, color);
	addLine(p6, color, p7, color);
}

void Wireframe::addSphere(Vector3 center, float radius, uint8_t resolution, Vector4 color)
{
	static const float pi = acos(-1.0f);
	float dphi = pi / resolution;
	float dtheta = dphi;

	for (int i = 1; i < resolution; i++) {
		float phi = i * dphi;
		float phi_1 = (i - 1) * dphi;

		for (int j = 0; j < resolution * 2; j++) {
			float theta = j * dtheta;
			float theta_1 = (j - 1) * dtheta;
			float ctheta = cos(theta), cphi = cos(phi);
			float stheta = sin(theta), sphi = sin(phi);
			float ctheta_1 = cos(theta_1), cphi_1 = cos(phi_1);
			float stheta_1 = sin(theta_1), sphi_1 = sin(phi_1);

			Vector3 p0 = center + Vector3(radius * ctheta * sphi, radius * stheta * sphi, radius * cphi);
			Vector3 p1 = center + Vector3(radius * ctheta * sphi_1, radius * stheta * sphi_1, radius * cphi_1);
			Vector3 p2 = center + Vector3(radius * ctheta_1 * sphi, radius * stheta_1 * sphi, radius * cphi);

			addLine(p0, color, p1, color);
			addLine(p0, color, p2, color);
		}
	}
	
	float phi = (resolution - 1) * dphi;

	Vector3 p1 = center + Vector3(0, 0, -radius);

	for (int j = 0; j < resolution * 2; j++) {
		float theta = j * dtheta;
		float theta_1 = (j + 1) * dtheta;

		float ctheta = cos(theta), cphi = cos(phi);
		float stheta = sin(theta), sphi = sin(phi);

		Vector3 p0 = center + Vector3(radius * ctheta * sphi, radius * stheta * sphi, radius * cphi);

		addLine(p0, color, p1, color);
	}
}

void Wireframe::addCylinder(Vector3 center, float radius, float extent, uint8_t resolution, Vector4 color)
{
	static const float tau = acos(-1.0f) * 2.0f;
	float dtheta = tau / resolution;

	Vector3 center1 = center + Vector3::J * extent;
	Vector3 center2 = center - Vector3::J * extent;

	for (int i = 0; i < resolution; i++) {
		float theta = i * dtheta;
		float theta_1 = (i + 1) * dtheta;
		float ctheta = cos(theta), ctheta_1 = cos(theta_1);
		float stheta = sin(theta), stheta_1 = sin(theta_1);

		Vector3 leg0 = Vector3(ctheta, 0, stheta) * radius;
		Vector3 leg1 = Vector3(ctheta_1, 0, stheta_1) * radius;

		Vector3 p0 = center1 + leg0;
		Vector3 p1 = center1 + leg1;
		Vector3 p2 = center2 + leg0;
		Vector3 p3 = center2 + leg1;

		addLine(center1, color, p0, color);
		addLine(p0, color, p1, color);
		addLine(p1, color, p3, color);
		addLine(p3, color, p2, color);
		addLine(p2, color, center2, color);
	}
}



void Wireframe::clear()
{
	num_lines = 0;
}

}
}
#endif