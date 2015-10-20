#pragma once

#if defined ( EAE6320_PLATFORM_GL )
#if defined ( _WIN32 )
// <windows.h> must be #included before <gl/GL.h>
#include "../Windows/WindowsIncludes.h"
#endif
#include <gl/GL.h>
#elif defined ( EAE6320_PLATFORM_D3D )
#include <d3d9.h>
#include <d3dx9shader.h>
#include <utility>
#else
#error "one of EAE6320_PLATFORM_GL or EAE6320_PLATFORM_D3D must be defined."
#endif

namespace eae6320
{
namespace Graphics
{
	struct Effect
	{
		typedef
#if defined( EAE6320_PLATFORM_GL )
			GLuint
#elif defined ( EAE6320_PLATFORM_D3D )
			LPDIRECT3DDEVICE9
#endif
			Parent;

		typedef
#if defined( EAE6320_PLATFORM_GL )
			GLint
#elif defined ( EAE6320_PLATFORM_D3D )
			std::pair<LPDIRECT3DVERTEXSHADER9, LPD3DXCONSTANTTABLE>
#endif
			VertexShader;

		typedef
#if defined( EAE6320_PLATFORM_GL )
			GLint
#elif defined ( EAE6320_PLATFORM_D3D )
			std::pair<LPDIRECT3DPIXELSHADER9, LPD3DXCONSTANTTABLE>
#endif
			FragmentShader;

		typedef
#if defined( EAE6320_PLATFORM_GL )
			GLuint
#elif defined ( EAE6320_PLATFORM_D3D )
			std::pair<LPD3DXBUFFER, LPD3DXCONSTANTTABLE>
#endif
			CompiledShader;

		typedef
#if defined( EAE6320_PLATFORM_GL )
			GLint
#elif defined ( EAE6320_PLATFORM_D3D )
			D3DXHANDLE
#endif
			PositionHandle;

		enum ShaderType
		{
			Vertex,
			Fragment
		};

		// for OpenGL, parent is the program (GLuint for program ID),
		//   composing both of the compiled shaders together.
		// for Direct3d, parent is the device (IDirect3DDevice9 *),
		//   which is necessary for most API functions.
		Parent parent;

		// for OpenGL, the shader IDs are only stored temporarily,
		//   then loaded into the program (parent) and zeroed out.
		// for Direct3D, these hold the actual pointers
		//   to the compiled and loaded shaders.
		VertexShader vertex_shader;
		FragmentShader fragment_shader;

		// this is a handle on the vertex shader's "g_position" uniform
		PositionHandle position_handle;


		static Effect * FromFiles(const char * vertexShaderPath, const char * fragmentShaderPath, Parent parent = 0);

		~Effect();
	};
}
}