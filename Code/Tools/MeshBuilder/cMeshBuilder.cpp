// Header Files
//=============

#include "cMeshBuilder.h"

#include <sstream>
#include <fstream>
#include "../../Engine/Windows/WindowsFunctions.h"
#include "../Debug_Buildtime/UserOutput.h"
#include "../../Engine/Graphics/Mesh.h"

// Interface
//==========

// Build
//------

bool eae6320::cMeshBuilder::Build( const std::vector<std::string>& )
{
	bool wereThereErrors = false;

	// Copy the source to the target
	{
		Mesh::Data * mesh_data = Mesh::Data::FromLuaFile(m_path_source);
		std::ofstream outfile;

		std::string errorMessage;
		if (mesh_data == NULL)
		{
			wereThereErrors = true;
			std::stringstream decoratedErrorMessage;
			decoratedErrorMessage << "Failed to build " << m_path_source << " to " << m_path_target;
			eae6320::UserOutput::Print(decoratedErrorMessage.str(), __FILE__);
			goto OnExit;
		}

		outfile.open(m_path_target, std::ofstream::binary);

		if (outfile.fail())
		{
			wereThereErrors = true;
			std::stringstream decoratedErrorMessage;
			decoratedErrorMessage << "Failed to open destination " << m_path_target;
			eae6320::UserOutput::Print(decoratedErrorMessage.str(), __FILE__);
			goto OnExit;
		}

		// THIS IS THE FORMAT DEFINITION
		// 4 bytes num_vertices (V)
		// 4 bytes num_triangles (T)
		// 12*V bytes vertices
		// 3*4*T bytes indices
		outfile.write(reinterpret_cast<char *>(&(mesh_data->num_vertices)),
			sizeof(mesh_data->num_vertices));
		outfile.write(reinterpret_cast<char *>(&(mesh_data->num_triangles)),
			sizeof(mesh_data->num_triangles));
		outfile.write(reinterpret_cast<char *>(mesh_data->vertices),
			mesh_data->num_vertices * sizeof(Mesh::Vertex));
		outfile.write(reinterpret_cast<char *>(mesh_data->indices),
			3 * mesh_data->num_triangles * sizeof(Mesh::Index));

		outfile.close();

		if (outfile.fail())
		{
			wereThereErrors = true;
			std::stringstream decoratedErrorMessage;
			decoratedErrorMessage << "Failed to write to " << m_path_target;
			eae6320::UserOutput::Print(decoratedErrorMessage.str(), __FILE__);
			goto OnExit;
		}

	OnExit:
		if (mesh_data)
			delete mesh_data;
		if (outfile.is_open())
			outfile.close();
	}
	
	return !wereThereErrors;
}
