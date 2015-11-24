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

#include <string>

namespace eae6320
{
namespace Graphics
{
	namespace ShaderTypes
	{
		enum eShaderType
		{
			Unknown,
			Vertex,
			Fragment
		};
	}

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
			std::pair<const DWORD *, LPD3DXCONSTANTTABLE>
#endif
			CompiledShader;

		typedef
#if defined( EAE6320_PLATFORM_GL )
			GLint
#elif defined ( EAE6320_PLATFORM_D3D )
			D3DXHANDLE
#endif
			UniformHandle;

		enum ShaderType
		{
			Vertex,
			Fragment
		};

		struct RenderState
		{
			bool
				alpha : 1,
				z_test : 1,
				z_write : 1,
				cull_back : 1;
		};

		struct Spec
		{
			std::string vertex_shd_path;
			std::string fragment_shd_path;
			RenderState flags;
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

		RenderState render_state;

		// this is a handle on the vertex shader's "g_position" uniform
		UniformHandle uni_local2world, uni_world2view, uni_view2screen;

		static Effect * FromFile(const char * effectPath, Parent parent = 0);
		static Effect * FromSpec(const Effect::Spec & spec, Parent parent = 0);

		~Effect();
	};
}
}