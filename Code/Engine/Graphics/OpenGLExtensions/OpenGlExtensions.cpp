// Header Files
//=============

#include "stdafx.h"

#include "OpenGlExtensions.h"

#include <sstream>
#include "../../Windows/WindowsFunctions.h"

// Helper Function Declarations
//=============================

namespace
{
	void* GetGlFunctionAddress( const char* i_functionName, std::string* o_errorMessage = NULL );
}

// Interface
//==========

// OpenGL Extension Definitions
//-----------------------------

PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
PFNGLBUFFERDATAPROC glBufferData = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
PFNGLDELETESHADERPROC glDeleteShader = NULL;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArray = NULL;
PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLUSEPROGRAMPROC glUseProgram = NULL;
PFNGLUNIFORM1FVPROC glUniform1fv = NULL;
PFNGLUNIFORM2FVPROC glUniform2fv = NULL;
PFNGLUNIFORM3FVPROC glUniform3fv = NULL;
PFNGLUNIFORM4FVPROC glUniform4fv = NULL;
PFNGLUNIFORM1IPROC glUniform1i = NULL;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D = NULL;

// Initialization
//---------------

bool eae6320::OpenGlExtensions::Load( std::string* o_errorMessage )
{
	// A current OpenGL context must exist before extensions can be loaded
	{
		const HGLRC currentContext = wglGetCurrentContext();
		if ( currentContext == NULL )
		{
			if ( o_errorMessage )
			{
				*o_errorMessage = "OpenGL extensions can't be loaded without a current OpenGL context";
			}
			return false;
		}
	}

#define EAE6320_LOADGLFUNCTION( i_functionName, i_functionType )														\
		i_functionName = reinterpret_cast<i_functionType>( GetGlFunctionAddress( #i_functionName, o_errorMessage ) );	\
		if ( !i_functionName )																							\
			return false;

	EAE6320_LOADGLFUNCTION( glActiveTexture, PFNGLACTIVETEXTUREPROC );
	EAE6320_LOADGLFUNCTION( glAttachShader, PFNGLATTACHSHADERPROC );
	EAE6320_LOADGLFUNCTION( glBindBuffer, PFNGLBINDBUFFERPROC );
	EAE6320_LOADGLFUNCTION( glBindVertexArray, PFNGLBINDVERTEXARRAYPROC );
	EAE6320_LOADGLFUNCTION( glBufferData, PFNGLBUFFERDATAPROC );
	EAE6320_LOADGLFUNCTION( glCompileShader, PFNGLCOMPILESHADERPROC );
	EAE6320_LOADGLFUNCTION( glCreateProgram, PFNGLCREATEPROGRAMPROC );
	EAE6320_LOADGLFUNCTION( glCreateShader, PFNGLCREATESHADERPROC );
	EAE6320_LOADGLFUNCTION( glDeleteBuffers, PFNGLDELETEBUFFERSPROC );
	EAE6320_LOADGLFUNCTION( glDeleteProgram, PFNGLDELETEPROGRAMPROC );
	EAE6320_LOADGLFUNCTION( glDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC );
	EAE6320_LOADGLFUNCTION( glDeleteShader, PFNGLDELETESHADERPROC );
	EAE6320_LOADGLFUNCTION( glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYARBPROC );
	EAE6320_LOADGLFUNCTION( glGenBuffers, PFNGLGENBUFFERSPROC );
	EAE6320_LOADGLFUNCTION( glGenVertexArrays, PFNGLGENVERTEXARRAYSPROC );
	EAE6320_LOADGLFUNCTION( glGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC );
	EAE6320_LOADGLFUNCTION( glGetProgramiv, PFNGLGETPROGRAMIVPROC );
	EAE6320_LOADGLFUNCTION( glGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC );
	EAE6320_LOADGLFUNCTION( glGetShaderiv, PFNGLGETSHADERIVPROC );
	EAE6320_LOADGLFUNCTION( glGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC );
	EAE6320_LOADGLFUNCTION( glLinkProgram, PFNGLLINKPROGRAMPROC );
	EAE6320_LOADGLFUNCTION( glShaderSource, PFNGLSHADERSOURCEPROC );
	EAE6320_LOADGLFUNCTION( glUniform1fv, PFNGLUNIFORM1FVPROC );
	EAE6320_LOADGLFUNCTION( glUniform2fv, PFNGLUNIFORM2FVPROC );
	EAE6320_LOADGLFUNCTION( glUniform3fv, PFNGLUNIFORM3FVPROC );
	EAE6320_LOADGLFUNCTION( glUniform4fv, PFNGLUNIFORM4FVPROC );
	EAE6320_LOADGLFUNCTION( glUniform1i, PFNGLUNIFORM1IPROC );
	EAE6320_LOADGLFUNCTION( glUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC );
	EAE6320_LOADGLFUNCTION( glUseProgram, PFNGLUSEPROGRAMPROC );
	EAE6320_LOADGLFUNCTION( glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC );
	EAE6320_LOADGLFUNCTION( glCompressedTexImage2D, PFNGLCOMPRESSEDTEXIMAGE2DPROC );

#undef EAE6320_LOADGLFUNCTION

	return true;
}

// Helper Function Declarations
//=============================

namespace
{
	void* GetGlFunctionAddress( const char* i_functionName, std::string* o_errorMessage )
	{
		void* address = reinterpret_cast<void*>( wglGetProcAddress( i_functionName ) );
		// The documentation says that NULL will be returned if the function isn't found,
		// but according to https://www.opengl.org/wiki/Load_OpenGL_Functions
		// other values can be returned by some implementations
		if ( ( address != NULL )
			&& ( address != reinterpret_cast<void*>( 1 ) ) && ( address != reinterpret_cast<void*>( 2 ) )
			&& ( address != reinterpret_cast<void*>( 3 ) ) && ( address != reinterpret_cast<void*>( -1 ) ) )
		{
			return address;
		}
		std::string wglErrorMessage;
		if ( address == NULL )
		{
			wglErrorMessage = eae6320::GetLastWindowsError();
		}
		// wglGetProcAddress() won't return the address of any 1.1 or earlier OpenGL functions
		// that are built into Windows' Opengl32.dll
		// but an attempt can be made to load those manually
		// in case the user of this function has made a mistake
		{
			// This library should already be loaded,
			// and so this function will just retrieve a handle to it
			HMODULE glLibrary = LoadLibrary( "Opengl32.dll" );
			if ( glLibrary != NULL )
			{
				// Look for an old OpenGL function
				void* address = reinterpret_cast<void*>( GetProcAddress( glLibrary, i_functionName ) );
				// Decrement the library's reference count
				FreeLibrary( glLibrary );
				// Return an address if it was found
				if ( address != NULL )
				{
					return address;
				}
			}
		}

		// If this code is reached the OpenGL function wasn't found
		if ( o_errorMessage )
		{
			std::stringstream errorMessage;
			errorMessage << "Windows failed to find the address of the OpenGL function \"" << i_functionName << "\"";
			if ( !wglErrorMessage.empty() )
			{
				errorMessage << ": " << wglErrorMessage;
			}
			*o_errorMessage = errorMessage.str();
		}

		return NULL;
	}
}
