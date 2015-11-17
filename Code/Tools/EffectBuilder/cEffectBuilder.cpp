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

bool LoadFXT(const char * in_path, Effect::Spec &spec)
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

			if (returnedValueCount == 1)
			{
				// A correct asset file _must_ return a table
				if (!lua_istable(luaState, -1))
				{
					std::stringstream errstr;
					errstr << "Asset files must return a table (instead of a " <<
						luaL_typename(luaState, -1) << ")\n";
					eae6320::UserOutput::Print(errstr.str());
					lua_pop(luaState, returnedValueCount);
					return false;
				}
			}
			else
			{
				std::stringstream errstr;
				errstr << "Asset files must return a single table (instead of " <<
					returnedValueCount << " values)"
					"\n";
				eae6320::UserOutput::Print(errstr.str());
				lua_pop(luaState, returnedValueCount);
				return false;
			}
		}
		else
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

		spec.vertex_shd_path = lua_tostring(luaState, -1);

		lua_pop(luaState, 1); // pop vertex shader path

		lua_getfield(luaState, -1, "fragment");

		if (!lua_isstring(luaState, -1))
		{
			eae6320::UserOutput::Print("Must have a path named 'fragment'\n");
			lua_pop(luaState, 2);
			return false;
		}

		spec.fragment_shd_path = lua_tostring(luaState, -1);
		lua_pop(luaState, 1); // pop fragment shader path

		lua_getfield(luaState, -1, "flags");

		//default render state
		spec.flags.alpha = false;
		spec.flags.z_test = true;
		spec.flags.z_write = true;
		spec.flags.cull_back = true;

		if (lua_istable(luaState, -1))
		{
			lua_getfield(luaState, -1, "alpha");
			if (lua_isboolean(luaState, -1))
				spec.flags.alpha = 0 != lua_toboolean(luaState, -1);
			lua_pop(luaState, 1);

			lua_getfield(luaState, -1, "z_test");
			if (lua_isboolean(luaState, -1))
				spec.flags.z_test = 0 != lua_toboolean(luaState, -1);
			lua_pop(luaState, 1);

			lua_getfield(luaState, -1, "z_write");
			if (lua_isboolean(luaState, -1))
				spec.flags.z_write = 0 != lua_toboolean(luaState, -1);
			lua_pop(luaState, 1);

			lua_getfield(luaState, -1, "cull_back");
			if (lua_isboolean(luaState, -1))
				spec.flags.cull_back = 0 != lua_toboolean(luaState, -1);
			lua_pop(luaState, 1);
		}

		lua_pop(luaState, 2); // pop flags, main table
		return true;
	}
}

bool SaveFXB(const char * out_path, const Effect::Spec & spec)
{
	std::ofstream outfile(out_path, std::ofstream::binary);

	if (outfile.fail())
	{
		std::stringstream decoratedErrorMessage;
		decoratedErrorMessage << "Failed to open destination " << out_path;
		eae6320::UserOutput::Print(decoratedErrorMessage.str(), __FILE__);
		return false;
	}

	// trick to convert the bit fields into a fixed char array, perfect for output
	// as long as there are 8 or fewer flags, the output will be 1 byte, etc.
	union { Effect::RenderState flags; char data[sizeof(Effect::RenderState)]; } converter;
	converter.flags = spec.flags;
	outfile.write(converter.data, sizeof(Effect::RenderState));

	uint16_t vertex_path_len = static_cast<uint16_t>(spec.vertex_shd_path.length() + 1);
	uint16_t fragment_path_len = static_cast<uint16_t>(spec.fragment_shd_path.length() + 1);

	outfile.write(reinterpret_cast<char *>(&vertex_path_len), sizeof(uint16_t));
	outfile.write(reinterpret_cast<char *>(&fragment_path_len), sizeof(uint16_t));
	outfile.write(spec.vertex_shd_path.c_str(), vertex_path_len);
	outfile.write(spec.fragment_shd_path.c_str(), fragment_path_len);

	outfile.close();

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
	Effect::Spec spec;

	return LoadFXT(m_path_source, spec) && SaveFXB(m_path_target, spec);
}
