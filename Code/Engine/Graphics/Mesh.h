#pragma once

#if defined ( EAE6320_PLATFORM_GL )
#include <gl/GL.h>
#elif defined ( EAE6320_PLATFORM_D3D )
#include <d3d9.h>
#else
#error "one of EAE6320_PLATFORM_GL or EAE6320_PLATFORM_D3D must be defined."
#endif
#include <cstdint>

namespace eae6320
{
	struct Mesh
	{
		uint32_t num_vertices;
		uint32_t num_triangles;

#if defined( EAE6320_PLATFORM_GL )
		GLuint gl_id;
#elif defined ( EAE6320_PLATFORM_D3D )
		IDirect3DVertexBuffer9 * vertex_buffer;
		IDirect3DIndexBuffer9 * index_buffer;
		IDirect3DVertexDeclaration9 * vertex_declaration;
#endif

		static Mesh * FromLua(const char * path);
	};
}