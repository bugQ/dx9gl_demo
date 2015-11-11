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
#include "Camera.h"
#include "../Math/Matrix4.h"

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
		void SetCamera(Effect & effect, Camera & camera);
		void SetEffect(Effect & effect, const Matrix4 local2world);
		void DrawMesh( Mesh & mesh );
		bool LoadMesh( Mesh & output, Mesh::Data & input );

		/* main graphics loop functions */
		void Clear();
		void BeginFrame();
		void DrawModel(Model & model, Camera & camera);
		void EndFrame();

		// must be called before quit
		bool ShutDown();
	}
}

#endif	// EAE6320_GRAPHICS_H
