// Header Files
//=============

#include "../cShaderBuilder.h"

#include <sstream>
#include "../../../Engine/Graphics/Includes.h"
#include "../../../Engine/Windows/Functions.h"

// Interface
//==========

// Build
//------

bool eae6320::cShaderBuilder::Build( const std::vector<std::string>& i_arguments )
{
	// Decide which kind of shader program to compile
	Graphics::ShaderTypes::eShaderType shaderType = Graphics::ShaderTypes::Unknown;
	{
		if ( i_arguments.size() >= 1 )
		{
			const std::string& argument = i_arguments[0];
			if ( argument == "vertex" )
			{
				shaderType = Graphics::ShaderTypes::Vertex;
			}
			else if ( argument == "fragment" )
			{
				shaderType = Graphics::ShaderTypes::Fragment;
			}
			else
			{
				std::stringstream errorMessage;
				errorMessage << "\"" << argument << "\" is not a valid shader program type";
				OutputErrorMessage( errorMessage.str().c_str(), m_path_source );
				return false;
			}
		}
		else
		{
			OutputErrorMessage(
				"A Shader must be built with an argument defining which type of shader program (e.g. \"vertex\" or \"fragment\") to compile",
				m_path_source );
			return false;
		}
	}
	// Get the path to the shader compiler
	std::string path_fxc;
	{
		// Get the path to the DirectX SDK
		std::string path_sdk;
		{
			std::string errorMessage;
			if ( !GetEnvironmentVariable( "DXSDK_DIR", path_sdk, &errorMessage ) )
			{
				std::stringstream decoratedErrorMessage;
				decoratedErrorMessage << "Windows failed to get the path to the DirectX SDK: " << errorMessage;
				OutputErrorMessage( decoratedErrorMessage.str().c_str(), __FILE__ );
				return false;
			}
		}
		path_fxc = path_sdk + "Utilities/bin/" +
#ifndef _WIN64
			"x86"
#else
			"x64"
#endif
			+ "/fxc.exe";
	}
	// Create the command to run
	std::string command;
	{
		std::stringstream commandToBuild;
		commandToBuild << "\"" << path_fxc << "\"";
		// Target profile
		switch ( shaderType )
		{
		case Graphics::ShaderTypes::Vertex:
			commandToBuild << " /Tvs_3_0";
			break;
		case Graphics::ShaderTypes::Fragment:
			commandToBuild << " /Tps_3_0";
			break;
		}
		// Entry point
		commandToBuild << " /Emain"
			// #define the platform
			<< " /DEAE6320_PLATFORM_D3D"
#ifdef EAE6320_GRAPHICS_AREDEBUGSHADERSENABLED
			// Disable optimizations so that debugging is easier
			<< " /Od"
			// Enable debugging
			<< " /Zi"
#endif
			// Target file
			<< " /Fo\"" << m_path_target << "\""
			// Don't output the logo
			<< " /nologo"
			// Source file
			<< " \"" << m_path_source << "\""
		;
		command = commandToBuild.str();
	}
	// Execute the command
	{
		DWORD exitCode;
		std::string errorMessage;
		if ( ExecuteCommand( command.c_str(), &exitCode, &errorMessage ) )
		{
			return exitCode == EXIT_SUCCESS;
		}
		else
		{
			OutputErrorMessage( errorMessage.c_str(), m_path_source );
			return false;
		}
	}
}
