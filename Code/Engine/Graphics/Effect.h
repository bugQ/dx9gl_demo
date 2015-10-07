#pragma once

#if defined ( EAE6320_PLATFORM_GL )
#if defined ( _WIN32 )
// <windows.h> must be #included before <gl/GL.h>
#include "../Windows/WindowsIncludes.h"
#endif
#include <gl/GL.h>
#elif defined ( EAE6320_PLATFORM_D3D )
#include <d3d9.h>
#else
#error "one of EAE6320_PLATFORM_GL or EAE6320_PLATFORM_D3D must be defined."
#endif

namespace eae6320
{
	struct Effect
	{
		typedef
#if defined( EAE6320_PLATFORM_GL )
			GLuint
#elif defined ( EAE6320_PLATFORM_D3D )
			IDirect3DDevice9 *
#endif
			Device;

		typedef
#if defined( EAE6320_PLATFORM_GL )
			GLint
#elif defined ( EAE6320_PLATFORM_D3D )
			IDirect3DVertexShader9 *
#endif
			VertexShader;

		typedef
#if defined( EAE6320_PLATFORM_GL )
			GLint
#elif defined ( EAE6320_PLATFORM_D3D )
			IDirect3DPixelShader9 *
#endif
			FragmentShader;

		typedef
#if defined( EAE6320_PLATFORM_GL )
			GLint
#elif defined ( EAE6320_PLATFORM_D3D )

			ID3DXBuffer *
#endif
			CompiledShader;


		Device device;
		VertexShader vertex_shader;
		FragmentShader fragment_shader;

		static Effect * FromFiles(const char * vertexShaderPath, const char * fragmentShaderPath, Device device = 0);
	};
}