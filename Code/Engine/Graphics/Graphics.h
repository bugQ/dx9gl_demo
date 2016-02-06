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
#include "Wireframe.h"
#include "../Math/Matrix4.h"

// Interface
//==========

namespace eae6320
{
	namespace Graphics
	{
		Effect::Parent GetDevice(); // used within Effect

		// must be called at startup
		bool Initialize( const HWND i_renderingWindow );

		/* used internally */
		void SetCamera(Effect & effect, Camera & camera);
		void SetRenderState(Effect::RenderState render_state);
		void SetEffect(Effect & effect);
		void SetMaterial(Material & material);
		void SetTransform(Effect & effect, const Matrix4 local2world);
		void DrawMesh( Mesh & mesh );
		bool LoadMesh( Mesh & output, Mesh::Data & input );

		/* debug draws*/
		bool InitWireframe( Wireframe & wireframe );
		bool BufferWireframe( Wireframe & wireframe );
		void DrawWireMesh( Mesh & wire_pool );
		void DrawWireframe( Wireframe & wireframe, Camera & camera ); // the kit&kaboodle

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
