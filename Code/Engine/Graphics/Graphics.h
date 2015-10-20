/*
	This file contains the function declarations for graphics
*/

#ifndef EAE6320_GRAPHICS_H
#define EAE6320_GRAPHICS_H

// Header Files
//=============

#include "../Windows/WindowsIncludes.h"
#include "Mesh.h"
#include "Effect.h"
#include "Model.h"
#include "../Math/Vector3.h"

// Interface
//==========

namespace eae6320
{
	namespace Graphics
	{
		bool Initialize( const HWND i_renderingWindow );
		void SetEffect( Effect & effect, Vector3 position );
		void DrawMesh( Mesh & mesh );
		Mesh LoadMesh( Mesh::Data & data );
		void DrawModel( Model & model );
		void Render();
		bool ShutDown();
	}
}

#endif	// EAE6320_GRAPHICS_H
