#pragma once

#if defined ( EAE6320_PLATFORM_GL )
#if defined ( _WIN32 )
// <windows.h> must be #included before <gl/GL.h>
#include "../Windows/WindowsIncludes.h"
#endif
#include <gl/GL.h>
#include <gl/GLU.h>
#include "OpenGLExtensions\OpenGlExtensions.h"
#elif defined ( EAE6320_PLATFORM_D3D )
#include <d3d9.h>
#else
#error "one of EAE6320_PLATFORM_GL or EAE6320_PLATFORM_D3D must be defined."
#endif
#include <cstdint>

namespace eae6320
{
namespace Graphics
{
	struct Mesh
	{
		struct Vertex
		{
			// POSITION
			// 3 floats == 12 bytes
			// Offset = 0
			float x, y, z;
			// COLOR0
			// 4 uint8_ts == 4 bytes
			// Offset = 12
#if defined ( EAE6320_PLATFORM_GL )
			uint8_t r, g, b, a; // OpenGL expects the byte layout of a color to be pretty much what you'd expect
#elif defined ( EAE6320_PLATFORM_D3D )
			uint8_t b, g, r, a;	// Direct3D expects the byte layout of a color to be different from what you might expect
#endif
			// TEXCOORDS0
			// 2 floats == 8 bytes
			// Offset = 16
			float u, v;
		};

		typedef uint32_t Index;

		// temporary struct for data to be passed to the graphics API
		// always right-handed triangle winding (counter-clockwise)
		// should always have 3*num_triangles indices
		struct Data
		{
			Vertex * vertices;
			Index * indices;
			uint32_t num_vertices;
			uint32_t num_triangles;

			Data();
			~Data();

			static Data * FromLuaFile(const char * path);
			static Data * FromBinFile(const char * path);
		};

		uint32_t num_vertices;
		uint32_t num_triangles;

#if defined( EAE6320_PLATFORM_GL )
		GLuint gl_id;
#elif defined ( EAE6320_PLATFORM_D3D )
		IDirect3DVertexBuffer9 * vertex_buffer;
		IDirect3DIndexBuffer9 * index_buffer;
#endif

		~Mesh();
	};
}
}