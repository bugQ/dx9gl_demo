#include "stdafx.h"
#include "Terrain.h"

namespace eae6320
{
namespace Physics
{

Terrain::Terrain(const char * collision_mesh_path)
	: collision_mesh(Graphics::Mesh::Data::FromBinFile(collision_mesh_path))
{
}


Terrain::~Terrain()
{
}

bool Terrain::SegmentCollides()
{
	return false;
}

}
}