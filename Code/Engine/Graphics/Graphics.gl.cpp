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

// Static Data Initialization
//===========================

namespace
{
	HWND s_renderingWindow = NULL;
	HDC s_deviceContext = NULL;
	HGLRC s_openGlRenderingContext = NULL;

	eae6320::Mesh * sa_meshes;
	unsigned int s_num_meshes;

	// OpenGL encapsulates a matching vertex shader and fragment shader into what it calls a "program".

	// A vertex shader is a program that operates on vertices.
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
	GLuint s_programId = 0;
}

// Helper Function Declarations
//=============================

namespace
{
	using namespace eae6320;

	bool CreateProgram();
	bool CreateRenderingContext();
	bool CreateVertexArray( Mesh & mesh, Mesh::Data & data );
	bool LoadAndAllocateShaderProgram( const char* i_path, void*& o_shader, size_t& o_size, std::string* o_errorMessage );
	bool LoadFragmentShader( const GLuint i_programId );
	bool LoadVertexShader( const GLuint i_programId );

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

	// Initialize meshes
	const unsigned int num_meshes = 2;
	sa_meshes = new Mesh[num_meshes];
	s_num_meshes = num_meshes;
	const char * const meshfilenames[num_meshes] = {
		"data/square.msh",
		"data/triangle.msh"
	};
	for (unsigned int i = 0; i < s_num_meshes; ++i)
	{
		Mesh::Data * data = Mesh::Data::FromFile(meshfilenames[i]);
		if (!data)
		{
			goto OnError;
		}
		if (!CreateVertexArray(sa_meshes[i], *data))
		{
			delete data;
			goto OnError;
		}
		delete data;
	}

	if ( !CreateProgram() )
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

void eae6320::Graphics::Render()
{
	// Every frame an entirely new image will be created.
	// Before drawing anything, then, the previous image will be erased
	// by "clearing" the image buffer (filling it with a solid color)
	{
		// Black is usually used
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		assert( glGetError() == GL_NO_ERROR );
		// In addition to the color, "depth" and "stencil" can also be cleared,
		// but for now we only care about color
		const GLbitfield clearColor = GL_COLOR_BUFFER_BIT;
		glClear( clearColor );
		assert( glGetError() == GL_NO_ERROR );
	}

	// The actual function calls that draw geometry
	{
		// Set the vertex and fragment shaders
		{
			glUseProgram( s_programId );
			assert( glGetError() == GL_NO_ERROR );
		}

		for (unsigned int i = 0; i < s_num_meshes; ++i)
			DrawMesh( sa_meshes[i] );
	}

	// Everything has been drawn to the "back buffer", which is just an image in memory.
	// In order to display it, the contents of the back buffer must be swapped with the "front buffer"
	// (which is what the user sees)
	{
		BOOL result = SwapBuffers( s_deviceContext );
		assert( result != FALSE );
	}
}

bool eae6320::Graphics::ShutDown()
{
	bool wereThereErrors = false;

	if ( s_openGlRenderingContext != NULL )
	{
		if ( s_programId != 0 )
		{
			glDeleteProgram( s_programId );
			const GLenum errorCode = glGetError();
			if ( errorCode != GL_NO_ERROR )
			{
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to delete the program: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				UserOutput::Print( errorMessage.str() );
			}
			s_programId = 0;
		}

		for (unsigned int i = 0; i < s_num_meshes; ++i)
		{
			if (sa_meshes[i].gl_id != 0)
			{
				const GLsizei arrayCount = 1;
				glDeleteVertexArrays(arrayCount, &sa_meshes[i].gl_id);
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to delete the vertex array: " <<
						reinterpret_cast<const char*>(gluErrorString(errorCode));
					UserOutput::Print(errorMessage.str());
				}
				sa_meshes[i].gl_id = 0;
			}
		}
		delete[] sa_meshes;

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
	bool CreateProgram()
	{
		// Create a program
		{
			s_programId = glCreateProgram();
			const GLenum errorCode = glGetError();
			if ( errorCode != GL_NO_ERROR )
			{
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to create a program: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
				return false;
			}
			else if ( s_programId == 0 )
			{
				eae6320::UserOutput::Print( "OpenGL failed to create a program" );
				return false;
			}
		}
		// Load and attach the shaders
		if ( !LoadVertexShader( s_programId ) )
		{
			return false;
		}
		if ( !LoadFragmentShader( s_programId ) )
		{
			return false;
		}
		// Link the program
		{
			glLinkProgram( s_programId );
			GLenum errorCode = glGetError();
			if ( errorCode == GL_NO_ERROR )
			{
				// Get link info
				// (this won't be used unless linking fails
				// but it can be useful to look at when debugging)
				std::string linkInfo;
				{
					GLint infoSize;
					glGetProgramiv( s_programId, GL_INFO_LOG_LENGTH, &infoSize );
					errorCode = glGetError();
					if ( errorCode == GL_NO_ERROR )
					{
						sLogInfo info( static_cast<size_t>( infoSize ) );
						GLsizei* dontReturnLength = NULL;
						glGetProgramInfoLog( s_programId, static_cast<GLsizei>( infoSize ), dontReturnLength, info.memory );
						errorCode = glGetError();
						if ( errorCode == GL_NO_ERROR )
						{
							linkInfo = info.memory;
						}
						else
						{
							std::stringstream errorMessage;
							errorMessage << "OpenGL failed to get link info of the program: " <<
								reinterpret_cast<const char*>( gluErrorString( errorCode ) );
							eae6320::UserOutput::Print( errorMessage.str() );
							return false;
						}
					}
					else
					{
						std::stringstream errorMessage;
						errorMessage << "OpenGL failed to get the length of the program link info: " <<
							reinterpret_cast<const char*>( gluErrorString( errorCode ) );
						eae6320::UserOutput::Print( errorMessage.str() );
						return false;
					}
				}
				// Check to see if there were link errors
				GLint didLinkingSucceed;
				{
					glGetProgramiv( s_programId, GL_LINK_STATUS, &didLinkingSucceed );
					errorCode = glGetError();
					if ( errorCode == GL_NO_ERROR )
					{
						if ( didLinkingSucceed == GL_FALSE )
						{
							std::stringstream errorMessage;
							errorMessage << "The program failed to link:\n" << linkInfo;
							eae6320::UserOutput::Print( errorMessage.str() );
							return false;
						}
					}
					else
					{
						std::stringstream errorMessage;
						errorMessage << "OpenGL failed to find out if linking of the program succeeded: " <<
							reinterpret_cast<const char*>( gluErrorString( errorCode ) );
						eae6320::UserOutput::Print( errorMessage.str() );
						return false;
					}
				}
			}
			else
			{
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to link the program: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
				return false;
			}
		}

		return true;
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
			glBufferData( GL_ARRAY_BUFFER, data.num_vertices * alignof(Mesh::Vertex),
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
			// 2 floats == 8 bytes
			// Offset = 0
			{
				const GLuint vertexElementLocation = 0;
				const GLint elementCount = 2;
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
			// Offset = 8
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
			glBufferData( GL_ELEMENT_ARRAY_BUFFER, data.num_triangles * 3 * alignof(Mesh::Index),
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

	bool LoadAndAllocateShaderProgram( const char* i_path, void*& o_shader, size_t& o_size, std::string* o_errorMessage )
	{
		bool wereThereErrors = false;

		// Load the shader source from disk
		o_shader = NULL;
		HANDLE fileHandle = INVALID_HANDLE_VALUE;
		{
			// Open the file
			{
				const DWORD desiredAccess = FILE_GENERIC_READ;
				const DWORD otherProgramsCanStillReadTheFile = FILE_SHARE_READ;
				SECURITY_ATTRIBUTES* useDefaultSecurity = NULL;
				const DWORD onlySucceedIfFileExists = OPEN_EXISTING;
				const DWORD useDefaultAttributes = FILE_ATTRIBUTE_NORMAL;
				const HANDLE dontUseTemplateFile = NULL;
				fileHandle = CreateFile( i_path, desiredAccess, otherProgramsCanStillReadTheFile,
					useDefaultSecurity, onlySucceedIfFileExists, useDefaultAttributes, dontUseTemplateFile );
				if ( fileHandle == INVALID_HANDLE_VALUE )
				{
					wereThereErrors = true;
					if ( o_errorMessage )
					{
						std::string err = eae6320::GetLastWindowsError();
						std::stringstream errorMessage;
						errorMessage << "Windows failed to open the shader file " << i_path << ": " << err ;
						*o_errorMessage = errorMessage.str();
					}
					goto OnExit;
				}
			}
			// Get the file's size
			{
				LARGE_INTEGER fileSize_integer;
				if ( GetFileSizeEx( fileHandle, &fileSize_integer ) != FALSE )
				{
					assert( fileSize_integer.QuadPart <= SIZE_MAX );
					o_size = static_cast<size_t>( fileSize_integer.QuadPart );
				}
				else
				{
					wereThereErrors = true;
					if ( o_errorMessage )
					{
						std::string err = eae6320::GetLastWindowsError();
						std::stringstream errorMessage;
						errorMessage << "Windows failed to get the size of shader: " << err;
						*o_errorMessage = errorMessage.str();
					}
					goto OnExit;
				}

				// Add an extra byte for a NULL terminator
				o_size += 1;
			}
			// Read the file's contents into temporary memory
			o_shader = malloc( o_size );
			if ( o_shader )
			{
				DWORD bytesReadCount;
				OVERLAPPED* readSynchronously = NULL;
				if ( ReadFile( fileHandle, o_shader, o_size,
					&bytesReadCount, readSynchronously ) == FALSE )
				{
					wereThereErrors = true;
					if ( o_errorMessage )
					{
						std::string err = eae6320::GetLastWindowsError();
						std::stringstream errorMessage;
						errorMessage << "Windows failed to read the contents of shader: " << err;
						*o_errorMessage = errorMessage.str();
					}
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				if ( o_errorMessage )
				{
					std::stringstream errorMessage;
					errorMessage << "Failed to allocate " << o_size << " bytes to read in the shader program " << i_path;
					*o_errorMessage = errorMessage.str();
				}
				goto OnExit;
			}
			// Add the NULL terminator
			reinterpret_cast<char *>(o_shader)[o_size - 1] = '\0';
		}

	OnExit:

		if ( wereThereErrors && o_shader )
		{
			free( o_shader );
			o_shader = NULL;
		}
		if ( fileHandle != INVALID_HANDLE_VALUE )
		{
			if ( CloseHandle( fileHandle ) == FALSE )
			{
				if ( !wereThereErrors && o_errorMessage )
				{
					std::string err = eae6320::GetLastWindowsError();
					std::stringstream errorMessage;
					errorMessage << "Windows failed to close the shader file handle: " << err;
					*o_errorMessage = errorMessage.str();
				}
				wereThereErrors = true;
			}
			fileHandle = INVALID_HANDLE_VALUE;
		}

		return !wereThereErrors;
	}

	bool LoadFragmentShader( const GLuint i_programId )
	{
		// Verify that compiling shaders at run-time is supported
		{
			GLboolean isShaderCompilingSupported;
			glGetBooleanv( GL_SHADER_COMPILER, &isShaderCompilingSupported );
			if ( !isShaderCompilingSupported )
			{
				eae6320::UserOutput::Print( "Compiling shaders at run-time isn't supported on this implementation (this should never happen)" );
				return false;
			}
		}

		bool wereThereErrors = false;

		// Load the source code from file and set it into a shader
		GLuint fragmentShaderId = 0;
		void* shaderSource = NULL;
		{
			// Load the shader source code
			size_t fileSize;
			{
				const char* sourceCodeFileName = "data/fragment.shd";
				std::string errorMessage;
				if ( !LoadAndAllocateShaderProgram( sourceCodeFileName, shaderSource, fileSize, &errorMessage ) )
				{
					wereThereErrors = true;
					eae6320::UserOutput::Print( errorMessage );
					goto OnExit;
				}
			}
			// Generate a shader
			fragmentShaderId = glCreateShader( GL_FRAGMENT_SHADER );
			{
				const GLenum errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					wereThereErrors = true;
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to get an unused fragment shader ID: " <<
						reinterpret_cast<const char*>( gluErrorString( errorCode ) );
					eae6320::UserOutput::Print( errorMessage.str() );
					goto OnExit;
				}
				else if ( fragmentShaderId == 0 )
				{
					wereThereErrors = true;
					eae6320::UserOutput::Print( "OpenGL failed to get an unused fragment shader ID" );
					goto OnExit;
				}
			}
			// Set the source code into the shader
			{
				const GLsizei shaderSourceCount = 2;
				const GLchar* shaderSources[shaderSourceCount] =
				{
					"#version 330 // first line, as required by GLSL\n"
					"#define EAE6320_PLATFORM_GL\n",
					reinterpret_cast<GLchar *>(shaderSource)
				};
				const GLint shaderSourceLengths[shaderSourceCount] =
				{
					static_cast<GLint>(strlen(shaderSources[0])),
					static_cast<GLint>(fileSize)
				};
				glShaderSource( fragmentShaderId, shaderSourceCount, shaderSources, NULL );
				const GLenum errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					wereThereErrors = true;
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to set the fragment shader source code: " <<
						reinterpret_cast<const char*>( gluErrorString( errorCode ) );
					eae6320::UserOutput::Print( errorMessage.str() );
					goto OnExit;
				}
			}
		}
		// Compile the shader source code
		{
			glCompileShader( fragmentShaderId );
			GLenum errorCode = glGetError();
			if ( errorCode == GL_NO_ERROR )
			{
				// Get compilation info
				// (this won't be used unless compilation fails
				// but it can be useful to look at when debugging)
				std::string compilationInfo;
				{
					GLint infoSize;
					glGetShaderiv( fragmentShaderId, GL_INFO_LOG_LENGTH, &infoSize );
					errorCode = glGetError();
					if ( errorCode == GL_NO_ERROR )
					{
						sLogInfo info( static_cast<size_t>( infoSize ) );
						GLsizei* dontReturnLength = NULL;
						glGetShaderInfoLog( fragmentShaderId, static_cast<GLsizei>( infoSize ), dontReturnLength, info.memory );
						errorCode = glGetError();
						if ( errorCode == GL_NO_ERROR )
						{
							compilationInfo = info.memory;
						}
						else
						{
							wereThereErrors = true;
							std::stringstream errorMessage;
							errorMessage << "OpenGL failed to get compilation info of the fragment shader source code: " <<
								reinterpret_cast<const char*>( gluErrorString( errorCode ) );
							eae6320::UserOutput::Print( errorMessage.str() );
							goto OnExit;
						}
					}
					else
					{
						wereThereErrors = true;
						std::stringstream errorMessage;
						errorMessage << "OpenGL failed to get the length of the fragment shader compilation info: " <<
							reinterpret_cast<const char*>( gluErrorString( errorCode ) );
						eae6320::UserOutput::Print( errorMessage.str() );
						goto OnExit;
					}
				}
				// Check to see if there were compilation errors
				GLint didCompilationSucceed;
				{
					glGetShaderiv( fragmentShaderId, GL_COMPILE_STATUS, &didCompilationSucceed );
					errorCode = glGetError();
					if ( errorCode == GL_NO_ERROR )
					{
						if ( didCompilationSucceed == GL_FALSE )
						{
							wereThereErrors = true;
							std::stringstream errorMessage;
							errorMessage << "The fragment shader failed to compile:\n" << compilationInfo;
							eae6320::UserOutput::Print( errorMessage.str() );
							goto OnExit;
						}
					}
					else
					{
						wereThereErrors = true;
						std::stringstream errorMessage;
						errorMessage << "OpenGL failed to find out if compilation of the fragment shader source code succeeded: " <<
							reinterpret_cast<const char*>( gluErrorString( errorCode ) );
						eae6320::UserOutput::Print( errorMessage.str() );
						goto OnExit;
					}
				}
			}
			else
			{
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to compile the fragment shader source code: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
				goto OnExit;
			}
		}
		// Attach the shader to the program
		{
			glAttachShader( i_programId, fragmentShaderId );
			const GLenum errorCode = glGetError();
			if ( errorCode != GL_NO_ERROR )
			{
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to attach the fragment shader to the program: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
				goto OnExit;
			}
		}

	OnExit:

		if ( fragmentShaderId != 0 )
		{
			// Even if the shader was successfully compiled
			// once it has been attached to the program we can (and should) delete our reference to it
			// (any associated memory that OpenGL has allocated internally will be freed
			// once the program is deleted)
			glDeleteShader( fragmentShaderId );
			const GLenum errorCode = glGetError();
			if ( errorCode != GL_NO_ERROR )
			{
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to delete the fragment shader ID: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
			}
			fragmentShaderId = 0;
		}
		if ( shaderSource != NULL )
		{
			free( shaderSource );
			shaderSource = NULL;
		}

		return !wereThereErrors;
	}

	bool LoadVertexShader( const GLuint i_programId )
	{
		// Verify that compiling shaders at run-time is supported
		{
			GLboolean isShaderCompilingSupported;
#if defined ( _WIN32 )
			// Win32 should always support this (GL 2.0 +).
			// The check is bypassed because it's not supported in Windows' GL API for some reason.
			isShaderCompilingSupported = GL_TRUE;
#else
			glGetBooleanv( GL_SHADER_COMPILER, &isShaderCompilingSupported );
#endif

			if ( !isShaderCompilingSupported )
			{
				eae6320::UserOutput::Print( "Compiling shaders at run-time isn't supported on this implementation (this should never happen)" );
				return false;
			}
		}

		bool wereThereErrors = false;

		// Load the source code from file and set it into a shader
		GLuint vertexShaderId = 0;
		void* shaderSource = NULL;
		{
			// Load the shader source code
			size_t fileSize;
			{
				const char* sourceCodeFileName = "data/vertex.shd";
				std::string errorMessage;
				if ( !LoadAndAllocateShaderProgram( sourceCodeFileName, shaderSource, fileSize, &errorMessage ) )
				{
					wereThereErrors = true;
					eae6320::UserOutput::Print( errorMessage );
					goto OnExit;
				}
			}

			// Generate a shader
			vertexShaderId = glCreateShader( GL_VERTEX_SHADER );
			{
				const GLenum errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					wereThereErrors = true;
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to get an unused vertex shader ID: " <<
						reinterpret_cast<const char*>( gluErrorString( errorCode ) );
					eae6320::UserOutput::Print( errorMessage.str() );
					goto OnExit;
				}
				else if ( vertexShaderId == 0 )
				{
					wereThereErrors = true;
					eae6320::UserOutput::Print( "OpenGL failed to get an unused vertex shader ID" );
					goto OnExit;
				}
			}
			// Set the source code into the shader
			{
				const GLsizei shaderSourceCount = 2;
				const GLchar* shaderSources[shaderSourceCount] =
				{
					"#version 330 // first line, as required by GLSL\n"
					"#define EAE6320_PLATFORM_GL\n",
					reinterpret_cast<GLchar *>( shaderSource )
				};
				const GLint shaderSourceLengths[shaderSourceCount] =
				{
					static_cast<GLint>(strlen(shaderSources[0])),
					static_cast<GLint>(fileSize)
				};
				glShaderSource( vertexShaderId, shaderSourceCount, shaderSources, NULL );
				const GLenum errorCode = glGetError();
				if ( errorCode != GL_NO_ERROR )
				{
					wereThereErrors = true;
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to set the vertex shader source code: " <<
						reinterpret_cast<const char*>( gluErrorString( errorCode ) );
					eae6320::UserOutput::Print( errorMessage.str() );
					goto OnExit;
				}
			}
		}
		// Compile the shader source code
		{
			glCompileShader( vertexShaderId );
			GLenum errorCode = glGetError();
			if ( errorCode == GL_NO_ERROR )
			{
				// Get compilation info
				// (this won't be used unless compilation fails
				// but it can be useful to look at when debugging)
				std::string compilationInfo;
				{
					GLint infoSize;
					glGetShaderiv( vertexShaderId, GL_INFO_LOG_LENGTH, &infoSize );
					errorCode = glGetError();
					if ( errorCode == GL_NO_ERROR )
					{
						sLogInfo info( static_cast<size_t>( infoSize ) );
						GLsizei* dontReturnLength = NULL;
						glGetShaderInfoLog( vertexShaderId, static_cast<GLsizei>( infoSize ), dontReturnLength, info.memory );
						errorCode = glGetError();
						if ( errorCode == GL_NO_ERROR )
						{
							compilationInfo = info.memory;
						}
						else
						{
							wereThereErrors = true;
							std::stringstream errorMessage;
							errorMessage << "OpenGL failed to get compilation info of the vertex shader source code: " <<
								reinterpret_cast<const char*>( gluErrorString( errorCode ) );
							eae6320::UserOutput::Print( errorMessage.str() );
							goto OnExit;
						}
					}
					else
					{
						wereThereErrors = true;
						std::stringstream errorMessage;
						errorMessage << "OpenGL failed to get the length of the vertex shader compilation info: " <<
							reinterpret_cast<const char*>( gluErrorString( errorCode ) );
						eae6320::UserOutput::Print( errorMessage.str() );
						goto OnExit;
					}
				}
				// Check to see if there were compilation errors
				GLint didCompilationSucceed;
				{
					glGetShaderiv( vertexShaderId, GL_COMPILE_STATUS, &didCompilationSucceed );
					errorCode = glGetError();
					if ( errorCode == GL_NO_ERROR )
					{
						if ( didCompilationSucceed == GL_FALSE )
						{
							wereThereErrors = true;
							std::stringstream errorMessage;
							errorMessage << "The vertex shader failed to compile:\n" << compilationInfo;
							eae6320::UserOutput::Print( errorMessage.str() );
							goto OnExit;
						}
					}
					else
					{
						wereThereErrors = true;
						std::stringstream errorMessage;
						errorMessage << "OpenGL failed to find out if compilation of the vertex shader source code succeeded: " <<
							reinterpret_cast<const char*>( gluErrorString( errorCode ) );
						eae6320::UserOutput::Print( errorMessage.str() );
						goto OnExit;
					}
				}
			}
			else
			{
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to compile the vertex shader source code: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
				goto OnExit;
			}
		}
		// Attach the shader to the program
		{
			glAttachShader( i_programId, vertexShaderId );
			const GLenum errorCode = glGetError();
			if ( errorCode != GL_NO_ERROR )
			{
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to attach the vertex shader to the program: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
				goto OnExit;
			}
		}

	OnExit:

		if ( vertexShaderId != 0 )
		{
			// Even if the shader was successfully compiled
			// once it has been attached to the program we can (and should) delete our reference to it
			// (any associated memory that OpenGL has allocated internally will be freed
			// once the program is deleted)
			glDeleteShader( vertexShaderId );
			const GLenum errorCode = glGetError();
			if ( errorCode != GL_NO_ERROR )
			{
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to delete the vertex shader ID: " <<
					reinterpret_cast<const char*>( gluErrorString( errorCode ) );
				eae6320::UserOutput::Print( errorMessage.str() );
			}
			vertexShaderId = 0;
		}
		if ( shaderSource != NULL )
		{
			free( shaderSource );
			shaderSource = NULL;
		}

		return !wereThereErrors;
	}
}
