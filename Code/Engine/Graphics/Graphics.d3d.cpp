// Header Files
//=============

#include "stdafx.h"

#include "Graphics.h"

#include <cassert>
#include <cstdint>
#include <d3d9.h>
#include <d3dx9shader.h>
#include <d3dx9core.h>
#include <sstream>
#include "../Debug_Runtime/UserOutput.h"
#include "Effect.h"

// Static Data Initialization
//===========================

namespace
{
	HWND s_renderingWindow = NULL;
	IDirect3D9* s_direct3dInterface = NULL;
	IDirect3DDevice9* s_direct3dDevice = NULL;
	IDirect3DVertexDeclaration9* s_standardVertexFormat = NULL;
	IDirect3DVertexBuffer9* s_spriteVertexBuffer = NULL;
	ID3DXFont* s_debugFont = NULL;
}

// Helper Function Declarations
//=============================

namespace
{
	using namespace eae6320::Graphics;

	eae6320::Matrix4 ScreenTransform(
		const float i_fieldOfView_y, const float i_aspectRatio,
		const float i_z_nearPlane, const float i_z_farPlane);
	bool CreateDevice();
	bool CreateIndexBuffer( Mesh & mesh, Mesh::Data & data );
	bool CreateInterface();
	bool CreateVertexBuffer( Mesh & mesh, Mesh::Data & data );
	HRESULT GetVertexProcessingUsage( DWORD& o_usage );
	bool LoadFragmentShader();
	bool LoadVertexShader();
}

// Interface
//==========
Effect::Parent eae6320::Graphics::GetDevice()
{
	return s_direct3dDevice;
}

bool eae6320::Graphics::LoadMesh(Mesh & output, Mesh::Data & input)
{
	return CreateVertexBuffer(output, input)
		&& CreateIndexBuffer(output, input);
}

bool eae6320::Graphics::Initialize( const HWND i_renderingWindow )
{
	s_renderingWindow = i_renderingWindow;

	// Initialize the interface to the Direct3D9 library
	if ( !CreateInterface() )
	{
		return false;
	}
	// Create an interface to a Direct3D device
	if ( !CreateDevice() )
	{
		goto OnError;
	}

	// enable depth testing
	{
		HRESULT result = s_direct3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		assert(SUCCEEDED(result));
		result = s_direct3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		assert(SUCCEEDED(result));
		result = s_direct3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		assert(SUCCEEDED(result));
	}

	// Initialize the vertex format
	{
		// These elements must match the Vertex layout struct exactly.
		// They instruct Direct3D how to match the binary data in the vertex buffer
		// to the input elements in a vertex shader
		// (by using D3DDECLUSAGE enums here and semantics in the shader,
		// so that, for example, D3DDECLUSAGE_POSITION here matches with POSITION in shader code).
		// Note that OpenGL uses arbitrarily assignable number IDs to do the same thing.
		D3DVERTEXELEMENT9 vertexElements[] =
		{
			// Stream 0

			// POSITION
			// 3 floats == 12 bytes
			// Offset = 0
			{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },

			// NORMAL
			// 3 floats == 12 bytes
			// Offset = 12
			{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },

			// COLOR0
			// D3DCOLOR == 4 bytes
			// Offset = 24
			{ 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },

			// TEXCOORDS0
			// 2 floats == 8 bytes
			// Offset = 28
			{ 0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },

			// The following marker signals the end of the vertex declaration
			D3DDECL_END()
		};

		HRESULT result = s_direct3dDevice->CreateVertexDeclaration(vertexElements, &s_standardVertexFormat);
		if (!SUCCEEDED(result))
		{
			eae6320::UserOutput::Print("Direct3D failed to create a Direct3D9 vertex declaration");
			return false;
		}
	}

	// Set the vertex declaration for all meshes
	{
		HRESULT result = s_direct3dDevice->SetVertexDeclaration(s_standardVertexFormat);
		assert(SUCCEEDED(result));
	}


	// Create vertex buffer only for sprites:
	{
		// The usage tells Direct3D how this vertex buffer will be used
		DWORD usage = 0;
		{
			// The type of vertex processing should match what was specified when the device interface was created with CreateDevice()
			const HRESULT result = GetVertexProcessingUsage(usage);
			if (FAILED(result))
			{
				return false;
			}
			// Our code will only ever write to the buffer
			usage |= D3DUSAGE_DYNAMIC;
			usage |= D3DUSAGE_WRITEONLY;
		}

		const unsigned int bufferSize = sizeof(Mesh::Vertex) * 4;
		// Create a vertex buffer
		{
			// We will define our own vertex format
			const DWORD useSeparateVertexDeclaration = 0;
			// Place the vertex buffer into memory that Direct3D thinks is the most appropriate
			const D3DPOOL useDefaultPool = D3DPOOL_DEFAULT;
			HANDLE* const notUsed = NULL;
			const HRESULT result = s_direct3dDevice->CreateVertexBuffer(bufferSize,
				usage, useSeparateVertexDeclaration, useDefaultPool, &s_spriteVertexBuffer, notUsed);
			if (FAILED(result))
			{
				eae6320::UserOutput::Print("Direct3D failed to create a vertex buffer");
				return false;
			}
		}
	}

#ifdef _DEBUG
	{
		const HRESULT result = D3DXCreateFont(s_direct3dDevice
			, 32 // font height
			, 0 // font width
			, FW_NORMAL // font weight
			, 1 // mip levels
			, false // italic
			, DEFAULT_CHARSET // charset
			, OUT_DEFAULT_PRECIS // output precision
			, ANTIALIASED_QUALITY // quality
			, DEFAULT_PITCH | FF_DONTCARE // pitch and family
			, "Consolas" // face name
			, &s_debugFont);

		if (FAILED(result))
		{
			eae6320::UserOutput::Print("Direct3D failed to initialize a font");
			return false;
		}
	}
#endif // _DEBUG

	return true;

OnError:
	ShutDown();
	return false;
}

void eae6320::Graphics::DrawMesh( Mesh & mesh )
{
	// Bind a specific vertex buffer to the device as a data source
	{
		// There can be multiple streams of data feeding the display adaptor at the same time
		const unsigned int streamIndex = 0;
		// It's possible to start streaming data in the middle of a vertex buffer
		const unsigned int bufferOffset = 0;
		// The "stride" defines how large a single vertex is in the stream of data
		const unsigned int bufferStride = sizeof(Mesh::Vertex);
		HRESULT result = s_direct3dDevice->SetStreamSource(streamIndex, mesh.vertex_buffer, bufferOffset, bufferStride);
		assert(SUCCEEDED(result));
	}
	// Bind a specific index buffer to the device as a data source
	{
		HRESULT result = s_direct3dDevice->SetIndices(mesh.index_buffer);
		assert(SUCCEEDED(result));
	}
	// Render objects from the current streams
	{
		// We are using triangles as the "primitive" type,
		// and we have defined the vertex buffer as a triangle list
		// (meaning that every triangle is defined by three vertices)
		const D3DPRIMITIVETYPE primitiveType = D3DPT_TRIANGLELIST;
		// It's possible to start rendering primitives in the middle of the stream
		const unsigned int indexOfFirstVertexToRender = 0;
		const unsigned int indexOfFirstIndexToUse = 0;
		// We are drawing a square
		const unsigned int vertexCountToRender = mesh.num_vertices;	// How vertices from the vertex buffer will be used?
		const unsigned int primitiveCountToRender = mesh.num_triangles;	// How many triangles will be drawn?
		HRESULT result = s_direct3dDevice->DrawIndexedPrimitive(primitiveType,
			indexOfFirstVertexToRender, indexOfFirstVertexToRender, vertexCountToRender,
			indexOfFirstIndexToUse, primitiveCountToRender);
		assert(SUCCEEDED(result));
	}
}

void eae6320::Graphics::DrawSpriteQuad(Sprite & sprite)
{
	// Normalize sprite coordinates to the narrower viewport dimension
	// i.e. ensure that [-1,1] in x and y is always visible and square
	Sprite::Rect xy = sprite.xy, & uv = sprite.uv;
	D3DVIEWPORT9 viewport;
	HRESULT result = sprite.mat->effect->parent->GetViewport(&viewport);
	assert(SUCCEEDED(result));
	float w = static_cast<float>(viewport.Width);
	float h = static_cast<float>(viewport.Height);
	if (w > h)
	{
		float aspect = h / w;
		xy.x0 *= aspect;
		xy.x1 *= aspect;
	}
	else
	{
		float aspect = w / h;
		xy.y0 *= aspect;
		xy.y1 *= aspect;
	}

	// Fill the vertex buffer with the triangle's vertices
	{
		// Before the vertex buffer can be changed it must be "locked"
		Mesh::Vertex * vertexData;
		{
			const unsigned int lockEntireBuffer = 0;
			const DWORD useDefaultLockingBehavior = D3DLOCK_DISCARD;
			const unsigned int bufferSize = sizeof(Mesh::Vertex) * 4;
			const HRESULT result = s_spriteVertexBuffer->Lock(lockEntireBuffer, bufferSize,
				reinterpret_cast<void**>(&vertexData), useDefaultLockingBehavior);
			assert(SUCCEEDED(result));
		}
		// Fill the buffer
		{
			Mesh::Vertex &v0 = vertexData[0];
			v0.position = Vector3(xy.x0, xy.y0, 1.0f);
			v0.r = v0.g = v0.b = v0.a = UINT8_MAX;
			v0.u = uv.x0; v0.v = uv.y0;

			Mesh::Vertex &v1 = vertexData[1];
			v1.position = Vector3(xy.x1, xy.y0, 1.0f);
			v1.r = v1.g = v1.b = v1.a = UINT8_MAX;
			v1.u = uv.x1; v1.v = uv.y0;

			Mesh::Vertex &v2 = vertexData[2];
			v2.position = Vector3(xy.x0, xy.y1, 1.0f);
			v2.r = v2.g = v2.b = v2.a = UINT8_MAX;
			v2.u = uv.x0; v2.v = uv.y1;

			Mesh::Vertex &v3 = vertexData[3];
			v3.position = Vector3(xy.x1, xy.y1, 1.0f);
			v3.r = v3.g = v3.b = v3.a = UINT8_MAX;
			v3.u = uv.x1; v3.v = uv.y1;
		}
		// The buffer must be "unlocked" before it can be used
		{
			const HRESULT result = s_spriteVertexBuffer->Unlock();
			assert(SUCCEEDED(result));
		}
	}
	// Bind a specific vertex buffer to the device as a data source
	{
		// There can be multiple streams of data feeding the display adaptor at the same time
		const unsigned int streamIndex = 0;
		// It's possible to start streaming data in the middle of a vertex buffer
		const unsigned int bufferOffset = 0;
		// The "stride" defines how large a single vertex is in the stream of data
		const unsigned int bufferStride = sizeof(Mesh::Vertex);
		HRESULT result = s_direct3dDevice->SetStreamSource(streamIndex, s_spriteVertexBuffer, bufferOffset, bufferStride);
		assert(SUCCEEDED(result));
	}
	// Render objects from the current streams
	{
		// We are using triangles as the "primitive" type,
		// and we have defined the vertex buffer as a line list
		// (meaning that every line is defined by two vertices)
		const D3DPRIMITIVETYPE primitiveType = D3DPT_TRIANGLESTRIP;
		// It's possible to start rendering primitives in the middle of the stream
		const unsigned int indexOfFirstVertexToRender = 0;
		// We are drawing a square
		const unsigned int primitiveCountToRender = 2;	// How many triangles will be drawn?
		HRESULT result = s_direct3dDevice->DrawPrimitive(primitiveType, indexOfFirstVertexToRender, primitiveCountToRender);
		assert(SUCCEEDED(result));
	}
}

#ifdef _DEBUG
bool eae6320::Graphics::InitWireframe(Wireframe & wireframe)
{
	// The usage tells Direct3D how this vertex buffer will be used
	DWORD usage = 0;
	{
		// The type of vertex processing should match what was specified when the device interface was created with CreateDevice()
		const HRESULT result = GetVertexProcessingUsage(usage);
		if (FAILED(result))
		{
			return false;
		}
		// Our code will only ever write to the buffer
		usage |= D3DUSAGE_WRITEONLY;
	}

	Mesh & mesh = *wireframe.mesh;

	const unsigned int bufferSize = sizeof(Mesh::Vertex) * Wireframe::MAXLINES * 2;
	// Create a vertex buffer
	{
		mesh.num_vertices = 0;
		// We will define our own vertex format
		const DWORD useSeparateVertexDeclaration = 0;
		// Place the vertex buffer into memory that Direct3D thinks is the most appropriate
		const D3DPOOL useDefaultPool = D3DPOOL_DEFAULT;
		HANDLE* const notUsed = NULL;
		const HRESULT result = s_direct3dDevice->CreateVertexBuffer(bufferSize,
			usage, useSeparateVertexDeclaration, useDefaultPool, &mesh.vertex_buffer, notUsed);
		if (FAILED(result))
		{
			eae6320::UserOutput::Print("Direct3D failed to create a vertex buffer");
			return false;
		}
	}

	return true;
}

bool eae6320::Graphics::BufferWireframe(Wireframe & wireframe)
{
	Mesh & mesh = *wireframe.mesh;

	// Fill the vertex buffer with the wireframe's vertices
	{
		// Before the vertex buffer can be changed it must be "locked"
		Mesh::Vertex * vertexData;
		{
			const unsigned int lockEntireBuffer = 0;
			const DWORD useDefaultLockingBehavior = 0;
			const HRESULT result = mesh.vertex_buffer->Lock(lockEntireBuffer, Wireframe::MAXLINES * 2,
				reinterpret_cast<void**>(&vertexData), useDefaultLockingBehavior);
			if (FAILED(result))
			{
				eae6320::UserOutput::Print("Direct3D failed to lock the vertex buffer");
				return false;
			}
		}
		// Fill the buffer
		{
			for (unsigned int i = 0; i < wireframe.num_lines * 2; ++i)
				vertexData[i] = wireframe.points[i];
			mesh.num_triangles = static_cast<uint32_t>(wireframe.num_lines);
			mesh.num_vertices = static_cast<uint32_t>(2 * wireframe.num_lines);
		}
		// The buffer must be "unlocked" before it can be used
		{
			const HRESULT result = mesh.vertex_buffer->Unlock();
			if (FAILED(result))
			{
				eae6320::UserOutput::Print("Direct3D failed to unlock the vertex buffer");
				return false;
			}
		}
	}

	return true;
}

void eae6320::Graphics::DrawWireMesh(Mesh & mesh)
{
	// Bind a specific vertex buffer to the device as a data source
	{
		// There can be multiple streams of data feeding the display adaptor at the same time
		const unsigned int streamIndex = 0;
		// It's possible to start streaming data in the middle of a vertex buffer
		const unsigned int bufferOffset = 0;
		// The "stride" defines how large a single vertex is in the stream of data
		const unsigned int bufferStride = sizeof(Mesh::Vertex);
		HRESULT result = s_direct3dDevice->SetStreamSource(streamIndex, mesh.vertex_buffer, bufferOffset, bufferStride);
		assert(SUCCEEDED(result));
	}
	// Render objects from the current streams
	{
		// We are using triangles as the "primitive" type,
		// and we have defined the vertex buffer as a line list
		// (meaning that every line is defined by two vertices)
		const D3DPRIMITIVETYPE primitiveType = D3DPT_LINELIST;
		// It's possible to start rendering primitives in the middle of the stream
		const unsigned int indexOfFirstVertexToRender = 0;
		// We are drawing a square
		const unsigned int primitiveCountToRender = mesh.num_triangles;	// How many triangles^Wlines will be drawn?
		HRESULT result = s_direct3dDevice->DrawPrimitive(primitiveType, indexOfFirstVertexToRender, primitiveCountToRender);
		assert(SUCCEEDED(result));
	}
}

// returns height of drawn text
int eae6320::Graphics::DrawDebugText(int x, int y, std::string & text)
{
	RECT rect;
	BOOL success = GetClientRect(s_renderingWindow, &rect);
	assert(success);

	rect.top = rect.bottom >> 5;
	rect.bottom = rect.bottom - rect.top;
	rect.left = rect.right >> 5;
	rect.right = rect.right - rect.left;

	rect.top += y;
	rect.left += x;

	int text_height = s_debugFont->DrawText(
		NULL, // sprite
		text.c_str(), // string
		-1, // count
		&rect, // rect
		DT_LEFT | DT_NOCLIP, // format
		0xffccff66); // color

	return text_height;
}
#endif

void eae6320::Graphics::SetCamera( Effect & effect, Camera & camera )
{
	HRESULT result;
	Matrix4 viewmat = Matrix4::Identity;
	viewmat.vec3(3) = -camera.position;
	viewmat = viewmat.dot(Matrix4::rotation_q(camera.rotation.inverse()));
	const D3DXMATRIX * mat2 = reinterpret_cast<const D3DXMATRIX *>(&viewmat);
	LPD3DXCONSTANTTABLE table = effect.vertex_shader.second;
	result = table->SetMatrixTranspose(s_direct3dDevice, effect.uni_world2view, mat2);
	assert(SUCCEEDED(result));
}

void eae6320::Graphics::SetRenderState( Effect::RenderState render_state )
{
	HRESULT result;

	if (render_state.alpha)
	{
		result = s_direct3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		assert(SUCCEEDED(result));
		result = s_direct3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		assert(SUCCEEDED(result));
		result = s_direct3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		assert(SUCCEEDED(result));
	}
	else
	{
		result = s_direct3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		assert(SUCCEEDED(result));
	}

	if (render_state.z_test)
	{
		result = s_direct3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		assert(SUCCEEDED(result));
		result = s_direct3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		assert(SUCCEEDED(result));
	}
	else
	{
		result = s_direct3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		assert(SUCCEEDED(result));
	}

	if (render_state.z_write)
	{
		result = s_direct3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		assert(SUCCEEDED(result));
	}
	else
	{
		result = s_direct3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		assert(SUCCEEDED(result));
	}

	if (render_state.cull_back)
	{
		result = s_direct3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		assert(SUCCEEDED(result));
	}
	else
	{
		result = s_direct3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		assert(SUCCEEDED(result));
	}
}

void eae6320::Graphics::SetEffect(Effect & effect)
{
	HRESULT result;

	SetRenderState(effect.render_state);

	result = s_direct3dDevice->SetVertexShader(effect.vertex_shader.first);
	assert(SUCCEEDED(result));
	result = s_direct3dDevice->SetPixelShader(effect.fragment_shader.first);
	assert(SUCCEEDED(result));
}

void eae6320::Graphics::SetTransform(Effect & effect, const Matrix4 local2world)
{
	HRESULT result;
	LPD3DXCONSTANTTABLE table = effect.vertex_shader.second;

	const D3DXMATRIX * mat1 = reinterpret_cast<const D3DXMATRIX *>(&local2world);
	result = table->SetMatrixTranspose(s_direct3dDevice, effect.uni_local2world, mat1);
	assert(SUCCEEDED(result));

	D3DVIEWPORT9 viewport;
	result = effect.parent->GetViewport(&viewport);
	assert(SUCCEEDED(result));
	float fov = std::atanf(1) * 4 / 3;
	float aspect = static_cast<float>(viewport.Width) / static_cast<float>(viewport.Height);
	Matrix4 screenmat = ScreenTransform(fov, aspect, 0.1f, 100.0f);
	const D3DXMATRIX * mat3 = reinterpret_cast<const D3DXMATRIX *>(&screenmat);
	result = table->SetMatrixTranspose(s_direct3dDevice, effect.uni_view2screen, mat3);
	assert(SUCCEEDED(result));
}

void eae6320::Graphics::Clear()
{
	const DWORD flags = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER;
	HRESULT result = s_direct3dDevice->Clear(0, NULL, flags, D3DCOLOR_XRGB(0, 0, 0), 1.0, 0);
	assert(SUCCEEDED(result));
}

void eae6320::Graphics::BeginFrame()
{
	HRESULT result = s_direct3dDevice->BeginScene();
	assert(SUCCEEDED(result));
}

void eae6320::Graphics::EndFrame()
{
	HRESULT result = s_direct3dDevice->EndScene();
	assert(SUCCEEDED(result));

	// Everything has been drawn to the "back buffer", which is just an image in memory.
	// In order to display it, the contents of the back buffer must be "presented"
	// (to the front buffer)
	{
		const RECT* noSourceRectangle = NULL;
		const RECT* noDestinationRectangle = NULL;
		const HWND useDefaultWindow = NULL;
		const RGNDATA* noDirtyRegion = NULL;
		HRESULT result = s_direct3dDevice->Present(noSourceRectangle, noDestinationRectangle, useDefaultWindow, noDirtyRegion);
		assert(SUCCEEDED(result));
	}
}

bool eae6320::Graphics::ShutDown()
{
	bool wereThereErrors = false;

#ifdef _DEBUG
	if (s_debugFont)
	{
		s_debugFont->Release();
		s_debugFont = NULL;
	}
#endif

	if ( s_direct3dInterface )
	{
		if ( s_direct3dDevice )
		{
			s_direct3dDevice->Release();
			s_direct3dDevice = NULL;
		}

		s_direct3dInterface->Release();
		s_direct3dInterface = NULL;
	}
	s_renderingWindow = NULL;

	return !wereThereErrors;
}

// Helper Function Definitions
//============================

namespace
{
	eae6320::Matrix4 ScreenTransform(
		const float i_fieldOfView_y, const float i_aspectRatio,
		const float i_z_nearPlane, const float i_z_farPlane)
	{
		const float yScale = 1.0f / std::tan(i_fieldOfView_y * 0.5f);
		const float xScale = yScale / i_aspectRatio;
		const float zDistanceScale = i_z_farPlane / (i_z_nearPlane - i_z_farPlane);
		return eae6320::Matrix4(
			xScale, 0.0f, 0.0f, 0.0f,
			0.0f, yScale, 0.0f, 0.0f,
			0.0f, 0.0f, zDistanceScale, -1.0f,
			0.0f, 0.0f, i_z_nearPlane * zDistanceScale, 0.0f);
	}

	bool CreateDevice()
	{
		const UINT useDefaultDevice = D3DADAPTER_DEFAULT;
		const D3DDEVTYPE useHardwareRendering = D3DDEVTYPE_HAL;
		const DWORD useHardwareVertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		D3DPRESENT_PARAMETERS presentationParameters = { 0 };
		{
			{
				const unsigned int useRenderingWindowDimensions = 0;
				presentationParameters.BackBufferWidth = useRenderingWindowDimensions;
				presentationParameters.BackBufferHeight = useRenderingWindowDimensions;
			}
			presentationParameters.BackBufferFormat = D3DFMT_X8R8G8B8;
			presentationParameters.BackBufferCount = 1;
			presentationParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
			presentationParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
			presentationParameters.hDeviceWindow = s_renderingWindow;
			presentationParameters.Windowed = TRUE;
			presentationParameters.EnableAutoDepthStencil = TRUE;
			presentationParameters.AutoDepthStencilFormat = D3DFMT_D16;
			presentationParameters.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		}
		HRESULT result = s_direct3dInterface->CreateDevice( useDefaultDevice, useHardwareRendering,
			s_renderingWindow, useHardwareVertexProcessing, &presentationParameters, &s_direct3dDevice );
		if ( SUCCEEDED( result ) )
		{
			return true;
		}
		else
		{
			eae6320::UserOutput::Print( "Direct3D failed to create a Direct3D9 device" );
			return false;
		}
	}

	bool CreateIndexBuffer( Mesh & mesh, Mesh::Data & data )
	{
		// The usage tells Direct3D how this vertex buffer will be used
		DWORD usage = 0;
		{
			// The type of vertex processing should match what was specified when the device interface was created with CreateDevice()
			const HRESULT result = GetVertexProcessingUsage( usage );
			if ( FAILED( result ) )
			{
				return false;
			}
			// Our code will only ever write to the buffer
			usage |= D3DUSAGE_WRITEONLY;
		}

		const unsigned int bufferSize = sizeof(Mesh::Index) * data.num_triangles * 3;
		// Create an index buffer
		{
			mesh.num_triangles = data.num_triangles;
			// We'll use 32-bit indices in this class to keep things simple
			// (i.e. every index will be a 32 bit unsigned integer)
			const D3DFORMAT format = D3DFMT_INDEX32;
			// Place the index buffer into memory that Direct3D thinks is the most appropriate
			const D3DPOOL useDefaultPool = D3DPOOL_DEFAULT;
			HANDLE* notUsed = NULL;
			const HRESULT result = s_direct3dDevice->CreateIndexBuffer( bufferSize,
				usage, format, useDefaultPool, &mesh.index_buffer, notUsed );
			if ( FAILED( result ) )
			{
				eae6320::UserOutput::Print( "Direct3D failed to create an index buffer" );
				return false;
			}
		}
		// Fill the index buffer with the triangles' connectivity data
		{
			// Before the index buffer can be changed it must be "locked"
			uint32_t* indexData;
			{
				const unsigned int lockEntireBuffer = 0;
				const DWORD useDefaultLockingBehavior = 0;
				const HRESULT result = mesh.index_buffer->Lock( lockEntireBuffer, bufferSize,
					reinterpret_cast<void**>( &indexData ), useDefaultLockingBehavior );
				if ( FAILED( result ) )
				{
					eae6320::UserOutput::Print( "Direct3D failed to lock the index buffer" );
					return false;
				}
			}
			// Fill the buffer
			{
				for (unsigned int i = 0; i < data.num_triangles; ++i) {
					indexData[i * 3] = data.indices[i * 3];
					indexData[i * 3 + 1] = data.indices[i * 3 + 2];
					indexData[i * 3 + 2] = data.indices[i * 3 + 1];
				}

			}
			// The buffer must be "unlocked" before it can be used
			{
				const HRESULT result = mesh.index_buffer->Unlock();
				if ( FAILED( result ) )
				{
					eae6320::UserOutput::Print( "Direct3D failed to unlock the index buffer" );
					return false;
				}
			}
		}

		return true;
	}

	bool CreateInterface()
	{
		// D3D_SDK_VERSION is #defined by the Direct3D header files,
		// and is just a way to make sure that everything is up-to-date
		s_direct3dInterface = Direct3DCreate9( D3D_SDK_VERSION );
		if ( s_direct3dInterface )
		{
			return true;
		}
		else
		{
			eae6320::UserOutput::Print( "DirectX failed to create a Direct3D9 interface" );
			return false;
		}
	}

	bool CreateVertexBuffer( Mesh & mesh, Mesh::Data & data )
	{
		// The usage tells Direct3D how this vertex buffer will be used
		DWORD usage = 0;
		{
			// The type of vertex processing should match what was specified when the device interface was created with CreateDevice()
			const HRESULT result = GetVertexProcessingUsage( usage );
			if ( FAILED( result ) )
			{
				return false;
			}
			// Our code will only ever write to the buffer
			usage |= D3DUSAGE_WRITEONLY;
		}

		const unsigned int bufferSize = sizeof(Mesh::Vertex) * data.num_vertices;
		// Create a vertex buffer
		{
			mesh.num_vertices = data.num_vertices;
			// We will define our own vertex format
			const DWORD useSeparateVertexDeclaration = 0;
			// Place the vertex buffer into memory that Direct3D thinks is the most appropriate
			const D3DPOOL useDefaultPool = D3DPOOL_DEFAULT;
			HANDLE* const notUsed = NULL;
			const HRESULT result = s_direct3dDevice->CreateVertexBuffer( bufferSize,
				usage, useSeparateVertexDeclaration, useDefaultPool, &mesh.vertex_buffer, notUsed );
			if ( FAILED( result ) )
			{
				eae6320::UserOutput::Print( "Direct3D failed to create a vertex buffer" );
				return false;
			}
		}
		// Fill the vertex buffer with the triangle's vertices
		{
			// Before the vertex buffer can be changed it must be "locked"
			Mesh::Vertex * vertexData;
			{
				const unsigned int lockEntireBuffer = 0;
				const DWORD useDefaultLockingBehavior = 0;
				const HRESULT result = mesh.vertex_buffer->Lock( lockEntireBuffer, bufferSize,
					reinterpret_cast<void**>( &vertexData ), useDefaultLockingBehavior );
				if ( FAILED( result ) )
				{
					eae6320::UserOutput::Print( "Direct3D failed to lock the vertex buffer" );
					return false;
				}
			}
			// Fill the buffer
			{
				for (unsigned int i = 0; i < data.num_vertices; ++i)
					vertexData[i] = data.vertices[i];
			}
			// The buffer must be "unlocked" before it can be used
			{
				const HRESULT result = mesh.vertex_buffer->Unlock();
				if ( FAILED( result ) )
				{
					eae6320::UserOutput::Print( "Direct3D failed to unlock the vertex buffer" );
					return false;
				}
			}
		}

		return true;
	}

	HRESULT GetVertexProcessingUsage( DWORD& o_usage )
	{
		D3DDEVICE_CREATION_PARAMETERS deviceCreationParameters;
		const HRESULT result = s_direct3dDevice->GetCreationParameters( &deviceCreationParameters );
		if ( SUCCEEDED( result ) )
		{
			DWORD vertexProcessingType = deviceCreationParameters.BehaviorFlags &
				( D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MIXED_VERTEXPROCESSING | D3DCREATE_SOFTWARE_VERTEXPROCESSING );
			o_usage = ( vertexProcessingType != D3DCREATE_SOFTWARE_VERTEXPROCESSING ) ? 0 : D3DUSAGE_SOFTWAREPROCESSING;
		}
		else
		{
			eae6320::UserOutput::Print( "Direct3D failed to get the device's creation parameters" );
		}
		return result;
	}
}
