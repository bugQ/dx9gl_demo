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
#ifdef EAE6320_PLATFORM_D3D
		Effect::Parent GetDevice(); // used within Effect
#endif

		// must be called at startup
		bool Initialize( const HWND i_renderingWindow );

		/* used internally */
		void SetEffect( Effect & effect, Vector3 position );
		void DrawMesh( Mesh & mesh );
		bool LoadMesh( Mesh & output, Mesh::Data & input );

		/* main graphics loop functions */
		void Clear();
		void BeginFrame();
		void DrawModel(Model & model);
		void EndFrame();

		// must be called before quit
		bool ShutDown();
	}
}

#endif	// EAE6320_GRAPHICS_H
