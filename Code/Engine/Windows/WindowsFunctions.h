/*
	This file contains wrappers for Windows functions
*/

#ifndef EAE6320_WINDOWSFUNCTIONS_H
#define EAE6320_WINDOWSFUNCTIONS_H

#include "WindowsIncludes.h"

#include <cstdint>
#include <string>

namespace eae6320
{
	// Interface
	//==========

	bool CopyFile( const char* const i_path_source, const char* const i_path_target,
		const bool i_shouldFunctionFailIfTargetAlreadyExists = false, const bool i_shouldTargetFileTimeBeModified = false,
		std::string* o_errorMessage = NULL );
	bool CreateDirectoryIfNecessary( const std::string& i_path, std::string* o_errorMessage = NULL );
	bool DoesFileExist( const char* const i_path, std::string* o_errorMessage = NULL, DWORD* o_errorCode = NULL );
	bool ExecuteCommand( const char* const i_command, DWORD* o_exitCode = NULL, std::string* o_errorMessage = NULL );
	bool ExecuteCommand( const char* const i_command, const char* const i_optionalArguments = NULL, DWORD* o_exitCode = NULL, std::string* o_errorMessage = NULL );
	bool GetEnvironmentVariable( const char* const i_key, std::string& o_value, std::string* o_errorMessage = NULL );
	std::string GetFormattedWindowsError( const DWORD i_errorCode );
	std::string GetLastWindowsError( DWORD* o_optionalErrorCode = NULL );
	bool GetLastWriteTime( const char* const i_path, uint64_t& o_lastWriteTime, std::string* o_errorMessage = NULL );
}

#endif	// EAE6320_WINDOWSFUNCTIONS_H
