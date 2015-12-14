// Header Files
//=============

#include "cMaterialBuilder.h"

#include <sstream>
#include <fstream>
#include "../../Engine/Windows/WindowsFunctions.h"
#include "../Debug_Buildtime/UserOutput.h"
#include "../../Engine/Graphics/Material.h"
#include "../../External/Lua/Includes.h"

// Interface
//==========

// Build
//------
using namespace eae6320::Graphics;

bool LoadMTT(const char * in_path, Material::Spec &spec)
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
		lua_getfield(luaState, -1, "effect");

		if (!lua_isstring(luaState, -1))
		{
			eae6320::UserOutput::Print("Must have a path named 'effect'\n");
			lua_pop(luaState, 2);
			return false;
		}

		spec.effect_path = lua_tostring(luaState, -1);

		lua_pop(luaState, 1); // pop vertex shader path

		lua_getfield(luaState, -1, "vertex_uniforms");  // uniforms table now at -1

		if (!lua_istable(luaState, -1))
		{
			eae6320::UserOutput::Print("Must have a table named 'vertex_uniforms'\n");
			lua_pop(luaState, 2);
			return false;
		}
		
		ptrdiff_t offset = 0;
		lua_pushnil(luaState); // uniforms table now at -2
		size_t i = 0;
		for (i; lua_next(luaState, -2) != 0; ++i) 
		{	// lua_next pops key, pushes key & value
			// uniforms table at -3, key (uniform name) at -2, value (uniform) at -1
			
			spec.params.resize(i + 1);
			spec.param_names.resize(i + 1);

			spec.params[i].handle = DIFF2UHANDLE(offset);

			spec.param_names[i] = lua_tostring(luaState, -2);
			spec.params[i].shaderType = Effect::ShaderType::Vertex;
			offset += spec.param_names[i].size() + 1;

			if (lua_istable(luaState, -1))
			{
				const int len = luaL_len(luaState, -1);
				if (len > 4 || len < 1)
					goto OnValueError;
				spec.params[i].vec_length = len;

				for (int j = 0; j < len; ++j)
				{
					lua_rawgeti(luaState, -1, j+1); // ... uniform at -2, element at -1
					if (!lua_isnumber(luaState, -1))
					{
						lua_pop(luaState, 1);
						goto OnValueError;
					}
					spec.params[i].vec[j] = static_cast<float>(lua_tonumber(luaState, -1));
					lua_pop(luaState, 1);
				}
			}
			else if (lua_isnumber(luaState, -1))
			{
				spec.params[i].vec_length = 1;
				spec.params[i].vec[0] = static_cast<float>(lua_tonumber(luaState, -1));
			}
			else
			{
			OnValueError:
				eae6320::UserOutput::Print(
					"Each uniform must be a single number or sequence of 1-4 numbers.");
				lua_pop(luaState, 4);
				return false;
			}

			lua_pop(luaState, 1);
		}

		lua_pop(luaState, 1); // pop vertex_uniforms

		lua_getfield(luaState, -1, "fragment_uniforms");  // uniforms table now at -1

		if (!lua_istable(luaState, -1))
		{
			eae6320::UserOutput::Print("Must have a table named 'fragment_uniforms'\n");
			lua_pop(luaState, 2);
			return false;
		}

		lua_pushnil(luaState); // uniforms table now at -2
		for (i; lua_next(luaState, -2) != 0; ++i)
		{	// lua_next pops key, pushes key & value
			// uniforms table at -3, key (uniform name) at -2, value (uniform) at -1

			spec.params.resize(i + 1);
			spec.param_names.resize(i + 1);

			spec.params[i].handle = DIFF2UHANDLE(offset);

			spec.param_names[i] = lua_tostring(luaState, -2);
			spec.params[i].shaderType = Effect::ShaderType::Fragment;
			offset += spec.param_names[i].size() + 1;

			if (lua_istable(luaState, -1))
			{
				const int len = luaL_len(luaState, -1);
				if (len > 4 || len < 1)
					goto OnValueError;
				spec.params[i].vec_length = len;

				for (int j = 0; j < len; ++j)
				{
					lua_rawgeti(luaState, -1, j + 1); // ... uniform at -2, element at -1
					if (!lua_isnumber(luaState, -1))
					{
						lua_pop(luaState, 1);
						goto OnValueError;
					}
					spec.params[i].vec[j] = static_cast<float>(lua_tonumber(luaState, -1));
					lua_pop(luaState, 1);
				}
			}
			else if (lua_isnumber(luaState, -1))
			{
				spec.params[i].vec_length = 1;
				spec.params[i].vec[0] = static_cast<float>(lua_tonumber(luaState, -1));
			}
			else
			{
			OnValueError:
				eae6320::UserOutput::Print(
					"Each uniform must be a single number or sequence of 1-4 numbers.");
				lua_pop(luaState, 4);
				return false;
			}

			lua_pop(luaState, 1);
		}
		spec.num_params = static_cast<uint16_t>(i);

		lua_pop(luaState, 2); // pop uniforms, main table
		return true;
	}
}

bool SaveMTB(const char * out_path, const Material::Spec & spec)
{
	std::ofstream outfile(out_path, std::ofstream::binary);

	if (outfile.fail())
	{
		std::stringstream decoratedErrorMessage;
		decoratedErrorMessage << "Failed to open destination " << out_path;
		eae6320::UserOutput::Print(decoratedErrorMessage.str(), __FILE__);
		return false;
	}

	// write the two numbers that determine how the file is partitioned
	// this will make seeking and loading data faster
	uint16_t effect_path_len = static_cast<uint16_t>(spec.effect_path.size() + 1);
	outfile.write(reinterpret_cast<const char *>(&effect_path_len), sizeof(uint16_t));
	// thus why num_params is written before effect_path
	outfile.write(reinterpret_cast<const char *>(&spec.num_params), sizeof(spec.num_params));

	outfile.write(spec.effect_path.c_str(), effect_path_len);

	for (size_t i = 0; i < spec.num_params; ++i)
		outfile.write(reinterpret_cast<const char *>(&spec.params[i]),
			sizeof(Material::UniformParameter));

	for (size_t i = 0; i < spec.num_params; ++i)
		outfile.write(spec.param_names[i].c_str(), spec.param_names[i].size() + 1);

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

bool eae6320::cMaterialBuilder::Build( const std::vector<std::string>& )
{
	Material::Spec spec;

	return LoadMTT(m_path_source, spec) && SaveMTB(m_path_target, spec);
}
