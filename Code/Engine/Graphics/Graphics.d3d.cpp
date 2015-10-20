// Header Files
//=============

#include "stdafx.h"

#include "Graphics.h"

#include <cassert>
#include <cstdint>
#include <d3d9.h>
#include <d3dx9shader.h>
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

	eae6320::Graphics::Mesh * sa_meshes;
	unsigned int s_num_meshes;

	// The vertex shader is a program that operates on vertices.
	// Its input comes from a C/C++ "draw call" and is:
	//	* Position
	//	* Any other data we want
	// Its output is:
	//	* Position
	//		(So that the graphics hardware knows which pixels to fill in for the triangle)
	//	* Any other data we want
	// The fragment shader is a program that operates on fragments
	// (or potential pixels).
	// Its input is:
	//	* The data that was output from the vertex shader,
	//		interpolated based on how close the fragment is
	//		to each vertex in the triangle.
	// Its output is:
	//	* The final color that the pixel should be
	eae6320::Graphics::Effect * s_effect;
}

// Helper Function Declarations
//=============================

namespace
{
	using namespace eae6320::Graphics;

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

	// Initialize meshes
	const unsigned int num_meshes = 2;
	sa_meshes = new Mesh[num_meshes];
	s_num_meshes = num_meshes;
	const char * const meshfilenames[num_meshes] = {
		"data/square.vib",
		"data/triangle.vib"
	};
	for (unsigned int i = 0; i < s_num_meshes; ++i)
	{
		Mesh::Data * data = Mesh::Data::FromBinFile(meshfilenames[i]);
		if (!data)
		{
			goto OnError;
		}
		if (!LoadMesh(sa_meshes[i], *data))
		{
			delete data;
			goto OnError;
		}
		delete data;
	}

	s_effect = Effect::FromFiles("data/vertex.shd", "data/fragment.shd", s_direct3dDevice);
	if (!s_effect)
	{
		goto OnError;
	}
	return true;

OnError:
	ShutDown();
	return false;
}

void eae6320::Graphics::DrawMesh( Mesh & mesh )
{
	// Set the vertex declaration for this mesh
	{
		HRESULT result = s_direct3dDevice->SetVertexDeclaration(mesh.vertex_declaration);
		assert(SUCCEEDED(result));
	}
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

void eae6320::Graphics::SetEffect(Effect & effect, Vector3 position)
{
	HRESULT result;
	result = s_direct3dDevice->SetVertexShader(effect.vertex_shader.first);
	assert(SUCCEEDED(result));
	result = s_direct3dDevice->SetPixelShader(effect.fragment_shader.first);
	assert(SUCCEEDED(result));
	const float pos[2] = { position.x, position.y };
	result = effect.vertex_shader.second->SetFloatArray(s_direct3dDevice, effect.position_handle, pos, 2);
	assert(SUCCEEDED(result));
}

void eae6320::Graphics::BeginFrame()
{
	// Every frame an entirely new image will be created.
	// Before drawing anything, then, the previous image will be erased
	// by "clearing" the image buffer (filling it with a solid color)
	{
		const D3DRECT* subRectanglesToClear = NULL;
		const DWORD subRectangleCount = 0;
		const DWORD clearTheRenderTarget = D3DCLEAR_TARGET;
		D3DCOLOR clearColor;
		{
			// Black is usually used:
			clearColor = D3DCOLOR_XRGB(0, 0, 0);
		}
		const float noZBuffer = 0.0f;
		const DWORD noStencilBuffer = 0;
		HRESULT result = s_direct3dDevice->Clear(subRectangleCount, subRectanglesToClear,
			clearTheRenderTarget, clearColor, noZBuffer, noStencilBuffer);
		assert(SUCCEEDED(result));
	}

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

	if ( s_direct3dInterface )
	{
		if ( s_direct3dDevice )
		{
			delete s_effect;
			s_effect = NULL;

			for (unsigned int i = 0; i < s_num_meshes; ++i)
			{
				if (sa_meshes[i].vertex_buffer)
				{
					sa_meshes[i].vertex_buffer->Release();
					sa_meshes[i].vertex_buffer = NULL;
				}
				if (sa_meshes[i].index_buffer)
				{
					sa_meshes[i].index_buffer->Release();
					sa_meshes[i].index_buffer = NULL;
				}
				if (sa_meshes[i].vertex_declaration)
				{
					s_direct3dDevice->SetVertexDeclaration(NULL);
					sa_meshes[i].vertex_declaration->Release();
					sa_meshes[i].vertex_declaration = NULL;
				}
			}
			delete[] sa_meshes;

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
			presentationParameters.EnableAutoDepthStencil = FALSE;
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

		// Initialize the vertex format
		{
			// These elements must match the VertexFormat::sVertex layout struct exactly.
			// They instruct Direct3D how to match the binary data in the vertex buffer
			// to the input elements in a vertex shader
			// (by using D3DDECLUSAGE enums here and semantics in the shader,
			// so that, for example, D3DDECLUSAGE_POSITION here matches with POSITION in shader code).
			// Note that OpenGL uses arbitrarily assignable number IDs to do the same thing.
			D3DVERTEXELEMENT9 vertexElements[] =
			{
				// Stream 0

				// POSITION
				// 2 floats == 8 bytes
				// Offset = 0
				{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },

				// COLOR0
				// D3DCOLOR == 4 bytes
				// Offset = 8
				{ 0, 8, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },

				// The following marker signals the end of the vertex declaration
				D3DDECL_END()
			};
			HRESULT result = s_direct3dDevice->CreateVertexDeclaration( vertexElements, &mesh.vertex_declaration);
			if ( ! SUCCEEDED( result ) )
			{
				eae6320::UserOutput::Print( "Direct3D failed to create a Direct3D9 vertex declaration" );
				return false;
			}
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
