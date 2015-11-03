// Header Files
//=============

#include "cEffectBuilder.h"

#include <sstream>
#include <fstream>
#include "../../Engine/Windows/WindowsFunctions.h"
#include "../Debug_Buildtime/UserOutput.h"
#include "../../Engine/Graphics/Effect.h"
#include "../../External/Lua/Includes.h"

// Interface
//==========

// Build
//------
using namespace eae6320::Graphics;

bool LoadFXT(const char * in_path,
	const char * &vertex_shd_path, const char * &fragment_shd_path)
{
	// Create a new Lua state
	lua_State* luaState = NULL;
	{
		luaState = luaL_newstate();
		if (!luaState)
		{
			eae6320::UserOutput::Print("Failed to create a new Lua state\n");
			return false;
		}
	}

	// Load the asset file as a "chunk",
	// meaning there will be a callable function at the top of the stack
	{
		const int luaResult = luaL_loadfile(luaState, in_path);
		if (luaResult != LUA_OK)
		{
			eae6320::UserOutput::Print(lua_tostring(luaState, -1));
			// Pop the error message
			lua_pop(luaState, 1);
			return false;
		}
	}

	// Read Lua data structures
	{
		lua_getfield(luaState, -1, "vertex");

		if (!lua_isstring(luaState, -1))
		{
			eae6320::UserOutput::Print("Must have a path named 'vertex'\n");
			lua_pop(luaState, 2);
			return false;
		}

		vertex_shd_path = lua_tostring(luaState, -1);

		lua_pop(luaState, 1); // pop vertex shader path

		lua_getfield(luaState, -1, "fragment");

		if (!lua_isstring(luaState, -1))
		{
			eae6320::UserOutput::Print("Must have a path named 'fragment'\n");
			delete vertex_shd_path;
			vertex_shd_path = NULL;
			lua_pop(luaState, 2);
			return false;
		}

		fragment_shd_path = lua_tostring(luaState, -1);
		lua_pop(luaState, 2);
		return true;
	}
}

bool SaveFXB(const char * out_path,
	const char * vertex_shd_path, const char * fragment_shd_path)
{
	std::ofstream outfile(out_path, std::ofstream::binary);

	if (outfile.fail())
	{
		std::stringstream decoratedErrorMessage;
		decoratedErrorMessage << "Failed to open destination " << out_path;
		eae6320::UserOutput::Print(decoratedErrorMessage.str(), __FILE__);
		return false;
	}

	uint16_t vertex_path_len = static_cast<uint16_t>(strlen(vertex_shd_path) + 1);
	uint16_t fragment_path_len = static_cast<uint16_t>(strlen(fragment_shd_path) + 1);

	outfile.write(reinterpret_cast<char *>(&vertex_path_len), sizeof(uint16_t));
	outfile.write(reinterpret_cast<char *>(&fragment_path_len), sizeof(uint16_t));
	outfile.write(vertex_shd_path, vertex_path_len);
	outfile.write(fragment_shd_path, fragment_path_len);

	if (outfile.fail())
	{
		std::stringstream decoratedErrorMessage;
		decoratedErrorMessage << "Failed to write to " << out_path;
		eae6320::UserOutput::Print(decoratedErrorMessage.str(), __FILE__);
		return false;
	}

	return true;
}

bool eae6320::cEffectBuilder::Build( const std::vector<std::string>& )
{
	bool wereThereErrors = false;

	const char * vertex_shd_path;
	const char * fragment_shd_path;

	if (!LoadFXT(m_path_source, vertex_shd_path, fragment_shd_path)) {
		wereThereErrors = true;
		goto OnExit;
	}

	if (!SaveFXB(m_path_target, vertex_shd_path, fragment_shd_path)) {
		wereThereErrors = true;
		goto OnExit;
	}

OnExit:
	delete vertex_shd_path;
	delete fragment_shd_path;
	return !wereThereErrors;
}
