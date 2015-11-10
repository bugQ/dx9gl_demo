// Header Files
//=============

#include "stdafx.h"

#include "Graphics.h"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <string>
#include <sstream>
#include "../Debug_Runtime/UserOutput.h"
#include "../Windows/WindowsFunctions.h"
#include "OpenGlExtensions/OpenGlExtensions.h"
#include "Effect.h"

// Static Data Initialization
//===========================

namespace
{
	HWND s_renderingWindow = NULL;
	HDC s_deviceContext = NULL;
	HGLRC s_openGlRenderingContext = NULL;
}

// Helper Function Declarations
//=============================

namespace
{
	using namespace eae6320::Graphics;

	eae6320::Matrix4 ScreenTransform(
		const float i_fieldOfView_y, const float i_aspectRatio,
		const float i_z_nearPlane, const float i_z_farPlane);

	bool CreateRenderingContext();
	bool CreateVertexArray( Mesh & mesh, Mesh::Data & data );

	// This helper struct exists to be able to dynamically allocate memory to get "log info"
	// which will automatically be freed when the struct goes out of scope
	struct sLogInfo
	{
		GLchar* memory;
		sLogInfo( const size_t i_size ) { memory = reinterpret_cast<GLchar*>( malloc( i_size ) ); }
		~sLogInfo() { if ( memory ) free( memory ); }
	};
}

// Interface
//==========

bool eae6320::Graphics::LoadMesh(Mesh & output, Mesh::Data & input)
{
	return CreateVertexArray(output, input);
}

bool eae6320::Graphics::Initialize( const HWND i_renderingWindow )
{
	s_renderingWindow = i_renderingWindow;

	// Create an OpenGL rendering context
	if ( !CreateRenderingContext() )
	{
		goto OnError;
	}

	// Load any required OpenGL extensions
	{
		std::string errorMessage;
		if ( !OpenGlExtensions::Load( &errorMessage ) )
		{
			UserOutput::Print( errorMessage );
			goto OnError;
		}
	}

	// don't draw tris that aren't facing camera
	{
		glEnable(GL_CULL_FACE);

		const GLenum errorCode = glGetError();
		switch (errorCode)
		{
		case GL_NO_ERROR:
			break;
		case GL_INVALID_ENUM:
			UserOutput::Print("GL_CULL_FACE is not supported by this driver ! :(");
			goto OnError;
		case GL_INVALID_OPERATION:
			UserOutput::Print("Someone called Initialize after BeginFrame ! WHy !!?");
			goto OnError;
		default:
			std::stringstream ss;
			ss << "OpenGL error code " << errorCode << " when trying to set GL_CULL_FACE.";
			UserOutput::Print(ss.str().c_str());
			goto OnError;
		}
	}

	// enable depth testing
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		assert(glGetError() == GL_NO_ERROR);
	}

	return true;

OnError:

	ShutDown();
	return false;
}

void eae6320::Graphics::DrawMesh( Mesh & mesh )
{
	// Bind a specific vertex buffer to the device as a data source
	{
		glBindVertexArray(mesh.gl_id);
		assert(glGetError() == GL_NO_ERROR);
	}
	// Render objects from the current streams
	{
		// We are using triangles as the "primitive" type,
		// and we have defined the vertex buffer as a triangle list
		// (meaning that every triangle is defined by three vertices)
		const GLenum mode = GL_TRIANGLES;
		// We'll use 32-bit indices in this class to keep things simple
		// (i.e. every index will be a 32 bit unsigned integer)
		const GLenum indexType = GL_UNSIGNED_INT;
		// It is possible to start rendering in the middle of an index buffer
		const GLvoid* const offset = 0;
		// We are drawing a square
		const GLsizei primitiveCountToRender = mesh.num_triangles;	// How many triangles will be drawn?
		const GLsizei vertexCountPerTriangle = 3;
		const GLsizei vertexCountToRender = primitiveCountToRender * vertexCountPerTriangle;
		glDrawElements(mode, vertexCountToRender, indexType, offset);
		const GLenum errorCode = glGetError();
		assert(errorCode == GL_NO_ERROR);
	}
}

void eae6320::Graphics::SetEffect( Effect & effect, const Matrix4 local2world )
{
	glUseProgram(effect.parent);
	const GLfloat * mat1 = reinterpret_cast<const GLfloat *>(&local2world);
	glUniformMatrix4fv(effect.uni_local2world, 1, false, mat1);
	assert(glGetError() == GL_NO_ERROR);

	Matrix4 viewmat = Matrix4::Identity;
	viewmat.vec3(3).z = -10;
	const GLfloat * mat2 = reinterpret_cast<const GLfloat *>(&viewmat);
	glUniformMatrix4fv(effect.uni_world2view, 1, false, mat2);
	assert(glGetError() == GL_NO_ERROR);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	float aspect = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);
	float fov = std::atan(1) * 4 / 3;
	Matrix4 screenmat = ScreenTransform(fov, aspect, 0.1f, 100.0f);
	const GLfloat * mat3 = reinterpret_cast<const GLfloat *>(&screenmat);
	glUniformMatrix4fv(effect.uni_view2screen, 1, false, mat3);
	assert(glGetError() == GL_NO_ERROR);
}

void eae6320::Graphics::Clear()
{
	// Black is usually used
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	assert(glGetError() == GL_NO_ERROR);
	// In addition to the color, "depth" and "stencil" can also be cleared,
	// but for now we only care about color
	const GLbitfield flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
	glClear(flags);
	assert(glGetError() == GL_NO_ERROR);
}

void eae6320::Graphics::BeginFrame()
{
}

void eae6320::Graphics::EndFrame()
{
	BOOL result = SwapBuffers(s_deviceContext);
	assert(result != FALSE);
}

bool eae6320::Graphics::ShutDown()
{
	bool wereThereErrors = false;

	if ( s_openGlRenderingContext != NULL )
	{
		if ( wglMakeCurrent( s_deviceContext, NULL ) != FALSE )
		{
			if ( wglDeleteContext( s_openGlRenderingContext ) == FALSE )
			{
				std::string err = GetLastWindowsError();
				std::stringstream errorMessage;
				errorMessage << "Windows failed to delete the OpenGL rendering context: " << err;
				UserOutput::Print( errorMessage.str() );
			}
		}
		else
		{
			std::string err = GetLastWindowsError();
			std::stringstream errorMessage;
			errorMessage << "Windows failed to unset the current OpenGL rendering context: " << err;
			UserOutput::Print( errorMessage.str() );
		}
		s_openGlRenderingContext = NULL;
	}

	if ( s_deviceContext != NULL )
	{
		// The documentation says that this call isn't necessary when CS_OWNDC is used
		ReleaseDC( s_renderingWindow, s_deviceContext );
		s_deviceContext = NULL;
	}

	s_renderingWindow = NULL;

	return !wereThereErrors;
}

// Helper Function Declarations
//=============================

namespace
{
	eae6320::Matrix4 ScreenTransform(
		const float i_fieldOfView_y, const float i_aspectRatio,
		const float i_z_nearPlane, const float i_z_farPlane)
	{
		const float yScale = 1.0f / std::tan(i_fieldOfView_y * 0.5f);
		const float xScale = yScale / i_aspectRatio;
		const float zDistanceScale = 1.0f / (i_z_nearPlane - i_z_farPlane);
		return eae6320::Matrix4(
			xScale, 0.0f, 0.0f, 0.0f,
			0.0f, yScale, 0.0f, 0.0f,
			0.0f, 0.0f, (i_z_nearPlane + i_z_farPlane) * zDistanceScale, -1.0f,
			0.0f, 0.0f, (2.0f * i_z_nearPlane * i_z_farPlane) * zDistanceScale, 0.0f);
	}

	bool CreateRenderingContext()
	{
		// A "device context" can be thought of an abstraction that Windows uses
		// to represent the graphics adaptor used to display a given window
		s_deviceContext = GetDC( s_renderingWindow );
		if ( s_deviceContext == NULL )
		{
			eae6320::UserOutput::Print( "Windows failed to get the device context" );
			return false;
		}
		// Windows requires that an OpenGL "render context" is made for the window we want to use to render into
		{
			// Set the pixel format of the rendering window
			{
				PIXELFORMATDESCRIPTOR desiredPixelFormat = { 0 };
				{
					desiredPixelFormat.nSize = sizeof( PIXELFORMATDESCRIPTOR );
					desiredPixelFormat.nVersion = 1;

					desiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
					desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
					desiredPixelFormat.cColorBits = 32;
					desiredPixelFormat.iLayerType = PFD_MAIN_PLANE ;
					desiredPixelFormat.cDepthBits = 16;
				}
				// Get the ID of the desired pixel format
				int pixelFormatId;
				{
					pixelFormatId = ChoosePixelFormat( s_deviceContext, &desiredPixelFormat );
					if ( pixelFormatId == 0 )
					{
						std::string err = eae6320::GetLastWindowsError();
						std::stringstream errorMessage;
						errorMessage << "Windows couldn't choose the closest pixel format: " << err;
						eae6320::UserOutput::Print( errorMessage.str() );
						return false;
					}
				}
				// Set it
				if ( SetPixelFormat( s_deviceContext, pixelFormatId, &desiredPixelFormat ) == FALSE )
				{
					std::string err = eae6320::GetLastWindowsError();
					std::stringstream errorMessage;
					errorMessage << "Windows couldn't set the desired pixel format: " << err;
					eae6320::UserOutput::Print( errorMessage.str() );
					return false;
				}
			}
			// Create the OpenGL rendering context
			s_openGlRenderingContext = wglCreateContext( s_deviceContext );
			if ( s_openGlRenderingContext == NULL )
			{
				std::string err = eae6320::GetLastWindowsError();
				std::stringstream errorMessage;
				errorMessage << "Windows failed to create an OpenGL rendering context: " << err;
				eae6320::UserOutput::Print( errorMessage.str() );
				return false;
			}
			// Set it as the rendering context of this thread
			if ( wglMakeCurrent( s_deviceContext, s_openGlRenderingContext ) == FALSE )
			{
				std::string err = eae6320::GetLastWindowsError();
				std::stringstream errorMessage;
				errorMessage << "Windows failed to set the current OpenGL rendering context: " << err;
				eae6320::UserOutput::Print( errorMessage.str() );
				return false;
			}
		}

		return true;
	}

	bool CreateVertexArray( Mesh & mesh, Mesh::Data & data )
	{
		bool wereThereErrors = false;
		GLuint vertexBufferId = 0;
		GLuint indexBufferId = 0;

		// Create a vertex array object and make it active
		{
			const GLsizei arrayCount = 1;
			glGenVertexArrays( arrayCount, &mesh.gl_id );
			const GLenum errorCode = glGetError();
			if ( errorCode == GL_NO_ERROR )
			{
				glBindVertexArray( mesh.gl_id );
				const GLenum errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					wereThereErrors = true;
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to bind the vertex array: " <<
						reinterpret_cast<const char*>( gluErrorString( errorCode ) );
					eae6320::UserOutput::Print( errorMessage.str() );
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to get an unused vertex array ID: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
				goto OnExit;
			}
		}

		// Create a vertex buffer object and make it active
		{
			const GLsizei bufferCount = 1;
			glGenBuffers( bufferCount, &vertexBufferId );
			const GLenum errorCode = glGetError();
			if ( errorCode == GL_NO_ERROR )
			{
				glBindBuffer( GL_ARRAY_BUFFER, vertexBufferId );
				const GLenum errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					wereThereErrors = true;
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to bind the vertex buffer: " <<
						reinterpret_cast<const char*>( gluErrorString( errorCode ) );
					eae6320::UserOutput::Print( errorMessage.str() );
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to get an unused vertex buffer ID: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
				goto OnExit;
			}
		}
		// Assign the data to the buffer
		{
			mesh.num_vertices = data.num_vertices;
			glBufferData( GL_ARRAY_BUFFER, data.num_vertices * sizeof(Mesh::Vertex),
				reinterpret_cast<GLvoid*>( data.vertices ),
				// Our code will only ever write to the buffer
				GL_STATIC_DRAW );
			const GLenum errorCode = glGetError();
			if ( errorCode != GL_NO_ERROR )
			{
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to allocate the vertex buffer: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
				goto OnExit;
			}
		}
		// Initialize the vertex format
		{
			const GLsizei stride = sizeof( Mesh::Vertex );
			GLvoid* offset = 0;

			// Position (0)
			// 3 floats == 12 bytes
			// Offset = 0
			{
				const GLuint vertexElementLocation = 0;
				const GLint elementCount = 3;
				const GLboolean notNormalized = GL_FALSE;	// The given floats should be used as-is
				glVertexAttribPointer( vertexElementLocation, elementCount, GL_FLOAT, notNormalized, stride, offset );
				const GLenum errorCode = glGetError();
				if ( errorCode == GL_NO_ERROR )
				{
					glEnableVertexAttribArray( vertexElementLocation );
					const GLenum errorCode = glGetError();
					if ( errorCode == GL_NO_ERROR )
					{
						offset = reinterpret_cast<GLvoid*>( reinterpret_cast<uint8_t*>( offset ) + ( elementCount * sizeof( float ) ) );
					}
					else
					{
						wereThereErrors = true;
						std::stringstream errorMessage;
						errorMessage << "OpenGL failed to enable the POSITION vertex attribute: " <<
							reinterpret_cast<const char*>( gluErrorString( errorCode ) );
						eae6320::UserOutput::Print( errorMessage.str() );
						goto OnExit;
					}
				}
				else
				{
					wereThereErrors = true;
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to set the POSITION vertex attribute: " <<
						reinterpret_cast<const char*>( gluErrorString( errorCode ) );
					eae6320::UserOutput::Print( errorMessage.str() );
					goto OnExit;
				}
			}
			// Color (1)
			// 4 uint8_ts == 4 bytes
			// Offset = 12
			{
				const GLuint vertexElementLocation = 1;
				const GLint elementCount = 4;
				// Each element will be sent to the GPU as an unsigned byte in the range [0,255]
				// but these values should be understood as representing [0,1] values
				// and that is what the shader code will interpret them as
				// (in other words, we could change the values provided here in C code
				// to be floats and sent GL_FALSE instead and the shader code wouldn't need to change)
				const GLboolean normalized = GL_TRUE;
				glVertexAttribPointer( vertexElementLocation, elementCount, GL_UNSIGNED_BYTE, normalized, stride, offset );
				const GLenum errorCode = glGetError();
				if ( errorCode == GL_NO_ERROR )
				{
					glEnableVertexAttribArray( vertexElementLocation );
					const GLenum errorCode = glGetError();
					if ( errorCode == GL_NO_ERROR )
					{
						offset = reinterpret_cast<GLvoid*>( reinterpret_cast<uint8_t*>( offset ) + ( elementCount * sizeof( uint8_t ) ) );
					}
					else
					{
						wereThereErrors = true;
						std::stringstream errorMessage;
						errorMessage << "OpenGL failed to enable the COLOR0 vertex attribute: " <<
							reinterpret_cast<const char*>( gluErrorString( errorCode ) );
						eae6320::UserOutput::Print( errorMessage.str() );
						goto OnExit;
					}
				}
				else
				{
					wereThereErrors = true;
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to set the COLOR0 vertex attribute: " <<
						reinterpret_cast<const char*>( gluErrorString( errorCode ) );
					eae6320::UserOutput::Print( errorMessage.str() );
					goto OnExit;
				}
			}
		}

		// Create an index buffer object and make it active
		{
			const GLsizei bufferCount = 1;
			glGenBuffers( bufferCount, &indexBufferId );
			const GLenum errorCode = glGetError();
			if ( errorCode == GL_NO_ERROR )
			{
				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBufferId );
				const GLenum errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					wereThereErrors = true;
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to bind the index buffer: " <<
						reinterpret_cast<const char*>( gluErrorString( errorCode ) );
					eae6320::UserOutput::Print( errorMessage.str() );
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to get an unused index buffer ID: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
				goto OnExit;
			}
		}
		// Allocate space and copy the triangle data into the index buffer
		{
			mesh.num_triangles = data.num_triangles;
			glBufferData( GL_ELEMENT_ARRAY_BUFFER, data.num_triangles * 3 * sizeof(Mesh::Index),
				reinterpret_cast<const GLvoid*>( data.indices ),
				// Our code will only ever write to the buffer
				GL_STATIC_DRAW );
			const GLenum errorCode = glGetError();
			if ( errorCode != GL_NO_ERROR )
			{
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to allocate the index buffer: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
				goto OnExit;
			}
		}

	OnExit:

		// Delete the buffer object
		// (the vertex array will hold a reference to it)
		if ( mesh.gl_id != 0 )
		{
			// Unbind the vertex array
			// (this must be done before deleting the vertex buffer)
			glBindVertexArray( 0 );
			const GLenum errorCode = glGetError();
			if ( errorCode == GL_NO_ERROR )
			{
				if ( vertexBufferId != 0 )
				{
					// NOTE: If you delete the vertex buffer object here, as I recommend,
					// then gDEBugger won't know about it and you won't be able to examine the data.
					// If you find yourself in a situation where you want to see the buffer object data in gDEBugger
					// comment out this block of code temporarily
					// (doing this will cause a memory leak so make sure to restore the deletion code after you're done debugging).
					const GLsizei bufferCount = 1;
					glDeleteBuffers( bufferCount, &vertexBufferId );
					const GLenum errorCode = glGetError();
					if ( errorCode != GL_NO_ERROR )
					{
						wereThereErrors = true;
						std::stringstream errorMessage;
						errorMessage << "OpenGL failed to delete the vertex buffer: " <<
							reinterpret_cast<const char*>( gluErrorString( errorCode ) );
						eae6320::UserOutput::Print( errorMessage.str() );
					}
					vertexBufferId = 0;
				}
				if ( indexBufferId != 0 )
				{
					// NOTE: See the same comment above about deleting the vertex buffer object here and gDEBugger
					// holds true for the index buffer
					const GLsizei bufferCount = 1;
					glDeleteBuffers( bufferCount, &indexBufferId );
					const GLenum errorCode = glGetError();
					if ( errorCode != GL_NO_ERROR )
					{
						wereThereErrors = true;
						std::stringstream errorMessage;
						errorMessage << "\nOpenGL failed to delete the index buffer: " <<
							reinterpret_cast<const char*>( gluErrorString( errorCode ) );
						eae6320::UserOutput::Print( errorMessage.str() );
					}
					indexBufferId = 0;
				}
			}
			else
			{
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to unbind the vertex array: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
			}
		}

		return !wereThereErrors;
	}
}
