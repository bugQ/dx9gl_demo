/*
	This file contains the function declarations for graphics
*/

#ifndef EAE6320_GRAPHICS_H
#define EAE6320_GRAPHICS_H

// Header Files
//=============

#include "Windows/WindowsIncludes.h"
#include "Mesh.h"

// Interface
//==========

namespace eae6320
{
	namespace Graphics
	{
		bool Initialize( const HWND i_renderingWindow );
		void DrawMesh( Mesh & mesh );
		void Render();
		bool ShutDown();
	}
}

#endif	// EAE6320_GRAPHICS_H
