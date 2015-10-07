#include "stdafx.h"
#include "Effect.h"

#include <sstream>
#include <cassert>
#include "../Windows/WindowsFunctions.h"
#include "../Debug_Runtime/UserOutput.h"

#if defined ( EAE6320_PLATFORM_GL )
#include <gl/GLU.h>
#include "OpenGlExtensions/OpenGlExtensions.h"
#elif defined ( EAE6320_PLATFORM_D3D )
#include <d3dx9shader.h>
#else
#error "one of EAE6320_PLATFORM_GL or EAE6320_PLATFORM_D3D must be defined."
#endif

namespace
{
	eae6320::Effect::Parent CreateParent();
	char * LoadAndAllocateShaderProgram(const char* i_path, size_t& o_size, std::string& o_errorMessage);
	eae6320::Effect::CompiledShader CompileShader(eae6320::Effect::Parent parent, const char * shaderStr, size_t size, const char *filename);
	eae6320::Effect::VertexShader LoadVertexShader(eae6320::Effect::Parent parent, eae6320::Effect::CompiledShader compiledShader);
	eae6320::Effect::FragmentShader LoadFragmentShader(eae6320::Effect::Parent parent, eae6320::Effect::CompiledShader compiledShader);


#if defined ( EAE6320_PLATFORM_GL )
	// This helper struct exists to be able to dynamically allocate memory to get "log info"
	// which will automatically be freed when the struct goes out of scope
	struct sLogInfo
	{
		GLchar* memory;
		sLogInfo(const size_t i_size) { memory = reinterpret_cast<GLchar*>(malloc(i_size)); }
		~sLogInfo() { if (memory) free(memory); }
	};
#endif
}

namespace eae6320
{
	Effect * Effect::FromFiles(const char * vertexShaderPath, const char * fragmentShaderPath, Parent parent)
	{
		Effect * effect = new Effect();
		effect->parent = parent ? parent : CreateParent();
		size_t vertex_shader_size, fragment_shader_size;

		std::string error_str;
		const char * vertex_shader_str = LoadAndAllocateShaderProgram(
			vertexShaderPath, vertex_shader_size, error_str);
		if (!vertex_shader_str && !error_str.empty()) {
			eae6320::UserOutput::Print(error_str, __FILE__);
			delete effect;
			return NULL;
		}
		effect->vertex_shader = LoadVertexShader(effect->parent, CompileShader(
			effect->parent, vertex_shader_str, vertex_shader_size, vertexShaderPath));

		const char * fragment_shader_str = LoadAndAllocateShaderProgram(
			vertexShaderPath, fragment_shader_size, error_str);
		if (!fragment_shader_str && !error_str.empty()) {
			eae6320::UserOutput::Print(error_str, __FILE__);
			delete effect;
			return NULL;
		}
		effect->fragment_shader = LoadFragmentShader(effect->parent, CompileShader(
			effect->parent, vertex_shader_str, vertex_shader_size, fragmentShaderPath));

		return effect;
	}
}

namespace
{
	char * LoadAndAllocateShaderProgram(const char* i_path, size_t& o_size, std::string& o_errorMessage)
	{
		bool wereThereErrors = false;
		char * o_shader;

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
				fileHandle = CreateFile(i_path, desiredAccess, otherProgramsCanStillReadTheFile,
					useDefaultSecurity, onlySucceedIfFileExists, useDefaultAttributes, dontUseTemplateFile);
				if (fileHandle == INVALID_HANDLE_VALUE)
				{
					wereThereErrors = true;
					std::string err = eae6320::GetLastWindowsError();
					std::stringstream errorMessage;
					errorMessage << "Windows failed to open the shader file " << i_path << ": " << err;
					o_errorMessage = errorMessage.str();
					goto OnExit;
				}
			}
			// Get the file's size
			{
				LARGE_INTEGER fileSize_integer;
				if (GetFileSizeEx(fileHandle, &fileSize_integer) != FALSE)
				{
					assert(fileSize_integer.QuadPart <= SIZE_MAX);
					o_size = static_cast<size_t>(fileSize_integer.QuadPart);
				}
				else
				{
					wereThereErrors = true;
					std::string err = eae6320::GetLastWindowsError();
					std::stringstream errorMessage;
					errorMessage << "Windows failed to get the size of shader: " << err;
					o_errorMessage = errorMessage.str();
					goto OnExit;
				}

				// Add an extra byte for a NULL terminator
				o_size += 1;
			}
			// Read the file's contents into temporary memory
			o_shader = static_cast<char *>(malloc(o_size));
			if (o_shader)
			{
				DWORD bytesReadCount;
				OVERLAPPED* readSynchronously = NULL;
				if (ReadFile(fileHandle, o_shader, static_cast<DWORD>(o_size),
					&bytesReadCount, readSynchronously) == FALSE)
				{
					wereThereErrors = true;
					std::string err = eae6320::GetLastWindowsError();
					std::stringstream errorMessage;
					errorMessage << "Windows failed to read the contents of shader: " << err;
					o_errorMessage = errorMessage.str();
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "Failed to allocate " << o_size << " bytes to read in the shader program " << i_path;
				o_errorMessage = errorMessage.str();
				goto OnExit;
			}
			// Add the NULL terminator
			reinterpret_cast<char *>(o_shader)[o_size - 1] = '\0';
		}

	OnExit:

		if (wereThereErrors && o_shader)
		{
			free(o_shader);
			o_shader = NULL;
		}
		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			if (CloseHandle(fileHandle) == FALSE)
			{
				if (!wereThereErrors)
				{
					std::string err = eae6320::GetLastWindowsError();
					std::stringstream errorMessage;
					errorMessage << "Windows failed to close the shader file handle: " << err;
					o_errorMessage = errorMessage.str();
				}
				wereThereErrors = true;
			}
			fileHandle = INVALID_HANDLE_VALUE;
		}

		return o_shader;
	}

#if defined ( EAE6320_PLATFORM_GL )

	eae6320::Effect::Parent CreateParent()
	{
		// Create a program
		GLuint program = glCreateProgram();
		const GLenum errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			std::stringstream errorMessage;
			errorMessage << "OpenGL failed to create a program: " <<
				reinterpret_cast<const char*>(gluErrorString(errorCode));
			eae6320::UserOutput::Print(errorMessage.str());
			return false;
		}
		else if (program == 0)
		{
			eae6320::UserOutput::Print("OpenGL failed to create a program");
			return false;
		}
		
		return program;
	}

	eae6320::Effect::CompiledShader CompileShader(eae6320::Effect::Parent program, const char * shaderStr, size_t size, const char *filename)
	{
		// Verify that compiling shaders at run-time is supported
		{
			GLboolean isShaderCompilingSupported;
			glGetBooleanv(GL_SHADER_COMPILER, &isShaderCompilingSupported);
			if (!isShaderCompilingSupported)
			{
				eae6320::UserOutput::Print("Compiling shaders at run-time isn't supported on this implementation (this should never happen)");
				return false;
			}
		}

		bool wereThereErrors = false;

		// Load the source code from file and set it into a shader
		GLuint shaderId = 0;
		{
			// Generate a shader
			shaderId = glCreateShader(GL_FRAGMENT_SHADER);
			{
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					wereThereErrors = true;
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to get an unused fragment shader ID: " <<
						reinterpret_cast<const char*>(gluErrorString(errorCode));
					eae6320::UserOutput::Print(errorMessage.str());
					goto OnExit;
				}
				else if (shaderId == 0)
				{
					wereThereErrors = true;
					eae6320::UserOutput::Print("OpenGL failed to get an unused fragment shader ID");
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
					reinterpret_cast<const GLchar *>(shaderStr)
				};
				const GLint shaderSourceLengths[shaderSourceCount] =
				{
					static_cast<GLint>(strlen(shaderSources[0])),
					static_cast<GLint>(size)
				};
				glShaderSource(shaderId, shaderSourceCount, shaderSources, NULL);
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					wereThereErrors = true;
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to set the fragment shader source code: " <<
						reinterpret_cast<const char*>(gluErrorString(errorCode));
					eae6320::UserOutput::Print(errorMessage.str());
					goto OnExit;
				}
			}
		}
		// Compile the shader source code
		{
			glCompileShader(shaderId);
			GLenum errorCode = glGetError();
			if (errorCode == GL_NO_ERROR)
			{
				// Get compilation info
				// (this won't be used unless compilation fails
				// but it can be useful to look at when debugging)
				std::string compilationInfo;
				{
					GLint infoSize;
					glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoSize);
					errorCode = glGetError();
					if (errorCode == GL_NO_ERROR)
					{
						sLogInfo info(static_cast<size_t>(infoSize));
						GLsizei* dontReturnLength = NULL;
						glGetShaderInfoLog(shaderId, static_cast<GLsizei>(infoSize), dontReturnLength, info.memory);
						errorCode = glGetError();
						if (errorCode == GL_NO_ERROR)
						{
							compilationInfo = info.memory;
						}
						else
						{
							wereThereErrors = true;
							std::stringstream errorMessage;
							errorMessage << "OpenGL failed to get compilation info of the fragment shader source code: " <<
								reinterpret_cast<const char*>(gluErrorString(errorCode));
							eae6320::UserOutput::Print(errorMessage.str());
							goto OnExit;
						}
					}
					else
					{
						wereThereErrors = true;
						std::stringstream errorMessage;
						errorMessage << "OpenGL failed to get the length of the fragment shader compilation info: " <<
							reinterpret_cast<const char*>(gluErrorString(errorCode));
						eae6320::UserOutput::Print(errorMessage.str());
						goto OnExit;
					}
				}
				// Check to see if there were compilation errors
				GLint didCompilationSucceed;
				{
					glGetShaderiv(shaderId, GL_COMPILE_STATUS, &didCompilationSucceed);
					errorCode = glGetError();
					if (errorCode == GL_NO_ERROR)
					{
						if (didCompilationSucceed == GL_FALSE)
						{
							wereThereErrors = true;
							std::stringstream errorMessage;
							errorMessage << "The fragment shader failed to compile:\n" << compilationInfo;
							eae6320::UserOutput::Print(errorMessage.str());
							goto OnExit;
						}
					}
					else
					{
						wereThereErrors = true;
						std::stringstream errorMessage;
						errorMessage << "OpenGL failed to find out if compilation of the fragment shader source code succeeded: " <<
							reinterpret_cast<const char*>(gluErrorString(errorCode));
						eae6320::UserOutput::Print(errorMessage.str());
						goto OnExit;
					}
				}
			}
			else
			{
				wereThereErrors = true;
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to compile the fragment shader source code: " <<
					reinterpret_cast<const char*>(gluErrorString(errorCode));
				eae6320::UserOutput::Print(errorMessage.str());
				goto OnExit;
			}
		}

	OnExit:
		if (shaderStr != NULL)
		{
			// free not providing a const argument form is a bug.  this cast is necessary.
			free((char *)shaderStr);
			shaderStr = NULL;
		}

		return !wereThereErrors;
	}

	eae6320::Effect::VertexShader LoadVertexShader(eae6320::Effect::Parent program, eae6320::Effect::CompiledShader compiledShader)
	{
		// Attach the shader to the program
		{
			glAttachShader(program, compiledShader);
			const GLenum errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to attach the vertex shader to the program: " <<
					reinterpret_cast<const char*>(gluErrorString(errorCode));
				eae6320::UserOutput::Print(errorMessage.str());
				goto OnExit;
			}
		}
	OnExit:
		if (compiledShader != 0)
		{
			// Even if the shader was successfully compiled
			// once it has been attached to the program we can (and should) delete our reference to it
			// (any associated memory that OpenGL has allocated internally will be freed
			// once the program is deleted)
			glDeleteShader(compiledShader);
			const GLenum errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to delete the vertex shader ID: " <<
					reinterpret_cast<const char*>(gluErrorString(errorCode));
				eae6320::UserOutput::Print(errorMessage.str());
			}
			compiledShader = 0;
		}
		return compiledShader;
	}


	eae6320::Effect::FragmentShader LoadFragmentShader(eae6320::Effect::Parent program, eae6320::Effect::CompiledShader compiledShader)
	{
		// Attach the shader to the program
		{
			glAttachShader(program, compiledShader);
			const GLenum errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to attach the fragment shader to the program: " <<
					reinterpret_cast<const char*>(gluErrorString(errorCode));
				eae6320::UserOutput::Print(errorMessage.str());
				goto OnExit;
			}
		}
	OnExit:
		if (compiledShader != 0)
		{
			// Even if the shader was successfully compiled
			// once it has been attached to the program we can (and should) delete our reference to it
			// (any associated memory that OpenGL has allocated internally will be freed
			// once the program is deleted)
			glDeleteShader(compiledShader);
			const GLenum errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to delete the fragment shader ID: " <<
					reinterpret_cast<const char*>(gluErrorString(errorCode));
				eae6320::UserOutput::Print(errorMessage.str());
			}
			compiledShader = 0;
		}
		return compiledShader;
	}

#elif defined ( EAE6320_PLATFORM_D3D )

	eae6320::Effect::Parent CreateParent()
	{
		assert(0);
		return NULL;
	}

	ID3DXBuffer * CompileShader(eae6320::Effect::Parent device, const char * shaderStr, size_t size, const char *filename)
	{
		// Load the source code from file and compile it
		ID3DXBuffer* compiledShader;
		{
			const D3DXMACRO macros[] =
			{
				{ "EAE6320_PLATFORM_D3D", "1" },
				{ NULL, NULL }
			};
			ID3DXInclude* includes = NULL;
			const char* entryPoint = "main";
			const char* profile = "vs_3_0";
			const DWORD noFlags = 0;
			ID3DXBuffer* errorMessages = NULL;
			ID3DXConstantTable** noConstants = NULL;
			HRESULT result = D3DXCompileShader(shaderStr, static_cast<UINT>(size),
				macros, includes, entryPoint, profile, noFlags,
				&compiledShader, &errorMessages, noConstants);
			if (SUCCEEDED(result))
			{
				if (errorMessages)
				{
					errorMessages->Release();
				}
			}
			else
			{
				if (errorMessages)
				{
					std::stringstream errorMessage;
					errorMessage << "Direct3D failed to compile the shader from the file " << filename
						<< ":\n" << reinterpret_cast<char*>(errorMessages->GetBufferPointer());
					eae6320::UserOutput::Print(errorMessage.str());
					errorMessages->Release();
				}
				else
				{
					std::stringstream errorMessage;
					errorMessage << "Direct3D failed to compile the shader from the file " << filename;
					eae6320::UserOutput::Print(errorMessage.str());
				}
				return NULL;
			}
			return compiledShader;
		}
	}

	eae6320::Effect::VertexShader LoadVertexShader(eae6320::Effect::Parent device, ID3DXBuffer * compiledShader)
	{
		IDirect3DVertexShader9 * shader;
		// Create the vertex shader object
		bool wereThereErrors = false;
		{
			const HRESULT result = device->CreateVertexShader(
				reinterpret_cast<DWORD*>(compiledShader->GetBufferPointer()),
				&shader);
			if (FAILED(result))
			{
				eae6320::UserOutput::Print("Direct3D failed to create the vertex shader");
				wereThereErrors = true;
			}
			compiledShader->Release();
		}
		return shader;
	}

	eae6320::Effect::FragmentShader LoadFragmentShader(eae6320::Effect::Parent device, ID3DXBuffer * compiledShader)
	{
		IDirect3DPixelShader9 * shader;
		// Create the vertex shader object
		bool wereThereErrors = false;
		{
			const HRESULT result = device->CreatePixelShader(
				reinterpret_cast<DWORD*>(compiledShader->GetBufferPointer()),
				&shader);
			if (FAILED(result))
			{
				eae6320::UserOutput::Print("Direct3D failed to create the fragment shader");
				wereThereErrors = true;
			}
			compiledShader->Release();
		}
		return shader;
	}

#endif
}
