#include "stdafx.h"
#include "Mesh.h"
#include "../../External/Lua/Includes.h"

#include "../Debug_Runtime/UserOutput.h"
#include <sstream>
#include <cassert>

namespace {
	using namespace eae6320;

	const int numVerticesPerPolygon = 3;

	int LoadVertices(lua_State &luaState, Mesh::Vertex* &vertices)
	{
		assert(vertices == NULL);

		int numVertices = -1;
		int depth = 0;

		const char * const key = "vertices";
		lua_getfield(&luaState, -1, key); // vertices array now at -1
		++depth;

		if (!lua_istable(&luaState, -1))
		{
			std::stringstream errstr;
			errstr << "The value at \"" << key << "\" must be a table "
				"(instead of a " << luaL_typename(&luaState, -1) << ")\n";
			UserOutput::Print(errstr.str());
			goto OnExit;
		}

		numVertices = luaL_len(&luaState, -1);
		vertices = new Mesh::Vertex[numVertices];

		++depth;
		for (int i = 0; i < numVertices; ++i)
		{
			lua_rawgeti(&luaState, -1, i+1); // vertex table now at -1
			++depth;

			if (!lua_istable(&luaState, -1))
			{
				std::stringstream errstr;
				errstr << "each element of " << key << " should be a table, not a "
					<< luaL_typename(&luaState, -1) << "\n";
				UserOutput::Print(errstr.str());
				goto OnExit;
			}

			const char * const poskey = "position";
			lua_getfield(&luaState, -1, poskey); // position array now at -1
			++depth;

			if (!lua_istable(&luaState, -1) || luaL_len(&luaState, -1) != 2)
			{
		OnBadPos:
				std::stringstream errstr;
				errstr << key << " must have " << poskey << " as a pair of numbers"
					<< luaL_typename(&luaState, -1) << "\n";
				UserOutput::Print(errstr.str());
				numVertices = -1;
				goto OnExit;
			}

			lua_rawgeti(&luaState, -1, 1); // x coordinate now at -1
			++depth;

			if (!lua_isnumber(&luaState, -1))
				goto OnBadPos;

			vertices[i].x = static_cast<float>(lua_tonumber(&luaState, -1));

			lua_pop(&luaState, 1); // pop x, leave position array
			--depth;

			lua_rawgeti(&luaState, -1, 2);// y coordinate now at -1
			++depth;

			if (!lua_isnumber(&luaState, -1))
				goto OnBadPos;

			vertices[i].y = (float)lua_tonumber(&luaState, -1);

			lua_pop(&luaState, 2); // pop y and position, leave vertex table
			depth -= 2;

			const char * const colorkey = "color";
			lua_getfield(&luaState, -1, colorkey); // color array now at -1
			++depth;

			if (!lua_istable(&luaState, -1) || luaL_len(&luaState, -1) != 4)
			{
		OnBadColor:
				std::stringstream errstr;
				errstr << key << " must have " << colorkey
					<< " as an RGBA quadruplet of [0.0, 1.0] numbers\n";
				UserOutput::Print(errstr.str());
				numVertices = -1;
				goto OnExit;
			}

			double rgba[4];

			++depth;
			for (int j = 0; j < 4; ++j)
			{
				lua_rawgeti(&luaState, -1, j+1); // color component now at -1

				if (!lua_isnumber(&luaState, -1))
					goto OnBadColor;

				rgba[j] = lua_tonumber(&luaState, -1);
				if (rgba[j] < 0.0 || rgba[j] > 1.0)
					goto OnBadColor;
				
				lua_pop(&luaState, 1);
			}
			--depth;

			vertices[i].r = static_cast<uint8_t>(rgba[0] * 255);
			vertices[i].g = static_cast<uint8_t>(rgba[1] * 255);
			vertices[i].b = static_cast<uint8_t>(rgba[2] * 255);
			vertices[i].a = static_cast<uint8_t>(rgba[3] * 255);

			lua_pop(&luaState, 2); // pop color and vertex, leave vertices
			depth -= 2;
		}
		--depth;

	OnExit:

		// Pop all temp values
		lua_pop(&luaState, depth);

		// cleanup on error
		if (numVertices < 0 && vertices != NULL) {
			delete[] vertices;
			vertices = NULL;
		}

		return numVertices;
	}

	int LoadIndices(lua_State &luaState, Mesh::Index* &indices)
	{
		assert(indices == NULL);

		int numIndices = -1;
		int depth = 0;

		char const * const key = "indices";
		lua_getfield(&luaState, -1, key); // indices array now at -1
		++depth;

		if (!lua_istable(&luaState, -1))
		{
			std::stringstream errstr;
			errstr << "The value at \"" << key << "\" must be a table "
				"(instead of a " << luaL_typename(&luaState, -1) << ")\n";
			UserOutput::Print(errstr.str());
			goto OnExit;
		}

		const int numPolygons = luaL_len(&luaState, -1);
		numIndices = numPolygons * numVerticesPerPolygon;
		indices = new Mesh::Index[numIndices];

		++depth;
		for (int i = 0; i < numPolygons; ++i)
		{
			lua_rawgeti(&luaState, -1, i+1); // index array (one polygon's worth) now at -1

			if (!lua_istable(&luaState, -1) || luaL_len(&luaState, -1) != numVerticesPerPolygon)
			{
				UserOutput::Print("Mesh indices must come in triples\n");
				numIndices = -1;
				goto OnExit;
			}

			++depth;
			for (int j = 0; j < numVerticesPerPolygon; ++j)
			{
				lua_rawgeti(&luaState, -1, j+1); // index now at -1

				if (!lua_isnumber(&luaState, -1))
				{
				OnBadIndex:
					std::stringstream errstr;
					errstr << "Indices must be natural numbers, not "
						<< luaL_typename(&luaState, -1) << "\n";
					UserOutput::Print(errstr.str());
					numIndices = -1;
					goto OnExit;
				}

				double index = lua_tonumber(&luaState, -1);

				if (index < 0 || index == std::numeric_limits<double>::infinity() || modf(index, &index) != 0)
					goto OnBadIndex;

				indices[i * numVerticesPerPolygon + j] = static_cast<Mesh::Index>(index);

				lua_pop(&luaState, 1); // pop index, leave index array
			}
			--depth;

			lua_pop(&luaState, 1); // pop index array, leave indices array
		}
		--depth;

	OnExit:

		// Pop all temp values
		lua_pop(&luaState, depth);

		// cleanup on error
		if (numIndices < 0 && indices != NULL)
		{
			delete[] indices;
			indices = NULL;
		}

		return numIndices;
	}

	bool FromLua(lua_State & luaState, Mesh::Data* &meshData)
	{
		assert(meshData == NULL);
		meshData = new Mesh::Data();

		meshData->num_vertices = LoadVertices(luaState, meshData->vertices);
		if (meshData->num_vertices < 0)
			goto OnExit;

		Mesh::Index * indices = NULL;
		meshData->num_triangles = LoadIndices(luaState, meshData->indices) / numVerticesPerPolygon;
		if (meshData->num_triangles < 0)
			goto OnExit;

	OnExit:
		if (meshData->num_triangles < 0) {
			delete meshData;
			meshData = NULL;
			return false;
		}
		return true;
	}

}
namespace eae6320
{
	Mesh::Data::Data()
		: vertices(NULL), indices(NULL), num_vertices(-1), num_triangles(-1)
	{
	}

	Mesh::Data::~Data()
	{
		delete[] vertices;
		vertices = NULL;
		delete[] indices;
		indices = NULL;
	}

	Mesh::Data * Mesh::Data::FromFile(const char* path)
	{
		Mesh::Data * meshData = NULL;

		// Create a new Lua state
		lua_State* luaState = NULL;
		{
			luaState = luaL_newstate();
			if (!luaState)
			{
				UserOutput::Print("Failed to create a new Lua state\n");
				goto OnExit;
			}
		}

		int depth = 0; // depth of stack, how many things to pop

		// Load the asset file as a "chunk",
		// meaning there will be a callable function at the top of the stack
		{
			const int luaResult = luaL_loadfile(luaState, path);
			if (luaResult != LUA_OK)
			{
				UserOutput::Print(lua_tostring(luaState, -1));
				// Pop the error message
				lua_pop(luaState, 1);
				goto OnExit;
			}
			++depth;
		}
		// Execute the "chunk", which should load the asset
		// into a table at the top of the stack
		{
			const int argumentCount = 0;
			const int returnValueCount = LUA_MULTRET;	// Return _everything_ that the file returns
			const int noMessageHandler = 0;
			const int luaResult = lua_pcall(luaState, argumentCount, returnValueCount, noMessageHandler);
			if (luaResult == LUA_OK)
			{
				// A well-behaved asset file will only return a single value
				const int returnedValueCount = lua_gettop(luaState);
				depth += returnedValueCount;

				if (returnedValueCount == 1)
				{
					// A correct asset file _must_ return a table
					if (!lua_istable(luaState, -1))
					{
						std::stringstream errstr;
						errstr << "Asset files must return a table (instead of a " <<
							luaL_typename(luaState, -1) << ")\n";
						UserOutput::Print(errstr.str());
						goto OnExit;
					}
				}
				else
				{
					std::stringstream errstr;
					errstr << "Asset files must return a single table (instead of " <<
						returnedValueCount << " values)"
						"\n";
					UserOutput::Print(errstr.str());
					goto OnExit;
				}
			}
			else
			{
				UserOutput::Print(lua_tostring(luaState, -1));
				// Pop the error message
				lua_pop(luaState, 1);
				goto OnExit;
			}
		}

		// If this code is reached the asset file was loaded successfully,
		// and its table is now at index -1
		FromLua(*luaState, meshData);

	OnExit:
		if (luaState)
		{
			lua_pop(luaState, depth);

			lua_close(luaState);
			luaState = NULL;
		}

		return meshData;
	}
}