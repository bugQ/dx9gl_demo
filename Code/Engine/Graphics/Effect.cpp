#include "stdafx.h"
#include "Effect.h"

#include <sstream>
#include <fstream>
#include <cassert>
#include "../Windows/WindowsFunctions.h"
#include "../Debug_Runtime/UserOutput.h"
#include "Graphics.h"

#if defined ( EAE6320_PLATFORM_GL )
#include <gl/GLU.h>
#include "OpenGlExtensions/OpenGlExtensions.h"
#elif defined ( EAE6320_PLATFORM_D3D )
#else
#error "one of EAE6320_PLATFORM_GL or EAE6320_PLATFORM_D3D must be defined."
#endif

namespace
{
	eae6320::Graphics::Effect::Parent CreateParent();
	char * LoadAndAllocateShaderProgram(const char* i_path, size_t& o_size, std::string& o_errorMessage);
	eae6320::Graphics::Effect::CompiledShader CompileShader(eae6320::Graphics::Effect::Parent parent, const char * shaderStr, size_t size, eae6320::Graphics::Effect::ShaderType type, const char *filename);
	eae6320::Graphics::Effect::VertexShader LoadVertexShader(eae6320::Graphics::Effect::Parent parent, eae6320::Graphics::Effect::CompiledShader compiledShader);
	eae6320::Graphics::Effect::FragmentShader LoadFragmentShader(eae6320::Graphics::Effect::Parent parent, eae6320::Graphics::Effect::CompiledShader compiledShader);
	bool FinishUp(eae6320::Graphics::Effect * effect);

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
namespace Graphics
{
	Effect * Effect::FromFile(
		const char * effectPath,
		Parent parent)
	{
		std::ifstream infile(effectPath, std::ifstream::binary);

		if (infile.fail())
		{
			std::stringstream errstr;
			errstr << "Could not open path " << effectPath << "\n";
			UserOutput::Print(errstr.str(), __FILE__);
			return NULL;
		}

		uint16_t vertex_path_len, fragment_path_len;
		char * vertex_shader_path, * fragment_shader_path;

		infile.read(reinterpret_cast<char *>(&vertex_path_len), sizeof(uint16_t));
		infile.read(reinterpret_cast<char *>(&fragment_path_len), sizeof(uint16_t));

		vertex_shader_path = new char[vertex_path_len];
		fragment_shader_path = new char[fragment_path_len];

		infile.read(vertex_shader_path, vertex_path_len);
		infile.read(fragment_shader_path, fragment_path_len);

		infile.close();

		if (infile.fail())
		{
			std::stringstream errstr;
			errstr << "Read error from path " << effectPath << "\n";
			UserOutput::Print(errstr.str(), __FILE__);
			return NULL;
		}

		Effect * effect = Effect::FromFiles(vertex_shader_path, fragment_shader_path);
		delete[] vertex_shader_path;
		delete[] fragment_shader_path;
		return effect;
	}

	Effect * Effect::FromFiles(
		const char * vertexShaderPath,
		const char * fragmentShaderPath,
		Parent parent)
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
		effect->vertex_shader = LoadVertexShader(effect->parent, CompileShader(effect->parent,
			vertex_shader_str, vertex_shader_size, eae6320::Graphics::Effect::ShaderType::Vertex, vertexShaderPath));

		const char * fragment_shader_str = LoadAndAllocateShaderProgram(
			fragmentShaderPath, fragment_shader_size, error_str);
		if (!fragment_shader_str && !error_str.empty()) {
			eae6320::UserOutput::Print(error_str, __FILE__);
			delete effect;
			return NULL;
		}
		effect->fragment_shader = LoadFragmentShader(effect->parent, CompileShader(effect->parent,
			fragment_shader_str, fragment_shader_size, eae6320::Graphics::Effect::ShaderType::Fragment, fragmentShaderPath));

		if (!FinishUp(effect))
		{
			delete effect;
			return NULL;
		}

		return effect;
	}

#if defined ( EAE6320_PLATFORM_GL )
	Effect::~Effect()
	{
		if (parent != 0)
		{
			glDeleteProgram(parent);
			const GLenum errorCode = glGetError();
			if (errorCode != GL_NO_ERROR)
			{
				std::stringstream errorMessage;
				errorMessage << "OpenGL failed to delete the program: " <<
					reinterpret_cast<const char*>(gluErrorString(errorCode));
				UserOutput::Print(errorMessage.str());
			}
			parent = 0;
		}
	}
#elif defined ( EAE6320_PLATFORM_D3D )
	Effect::~Effect()
	{
		if (vertex_shader.second) // constant table
			vertex_shader.second->Release();
		if (vertex_shader.first) // shader
			vertex_shader.first->Release();
		if (fragment_shader.second) // constant table
			fragment_shader.second->Release();
		if (fragment_shader.first) // shader
			fragment_shader.first->Release();
	}
#endif
}
}

namespace
{
	char * LoadAndAllocateShaderProgram(
		const char* i_path,
		size_t& o_size,
		std::string& o_errorMessage)
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

	eae6320::Graphics::Effect::Parent CreateParent()
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

	eae6320::Graphics::Effect::CompiledShader CompileShader(
		eae6320::Graphics::Effect::Parent program,
		const char * shaderStr,
		size_t size,
		eae6320::Graphics::Effect::ShaderType type,
		const char *filename)
	{
		// Verify that compiling shaders at run-time is supported
		{
			GLboolean isShaderCompilingSupported;
			glGetBooleanv(GL_SHADER_COMPILER, &isShaderCompilingSupported);
			if (!isShaderCompilingSupported)
			{
				eae6320::UserOutput::Print("Compiling shaders at run-time isn't supported on this implementation (this should never happen)");
				return 0;
			}
		}

		// Load the source code from file and set it into a shader
		GLuint shaderId = 0;
		{
			// Generate a shader
			GLenum shaderType;
			switch (type)
			{
			case eae6320::Graphics::Effect::ShaderType::Vertex:
				shaderType = GL_VERTEX_SHADER;
				break;
			case eae6320::Graphics::Effect::ShaderType::Fragment:
				shaderType = GL_FRAGMENT_SHADER;
				break;
			default:
				std::stringstream errorMessage;
				errorMessage << "Invalid ShaderType " << static_cast<int>(type);
				eae6320::UserOutput::Print(errorMessage.str());
				return 0;
			}
			shaderId = glCreateShader(shaderType);
			{
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to get an unused fragment shader ID: " <<
						reinterpret_cast<const char*>(gluErrorString(errorCode));
					eae6320::UserOutput::Print(errorMessage.str());
					goto OnExit;
				}
				else if (shaderId == 0)
				{
					eae6320::UserOutput::Print("OpenGL failed to get an unused fragment shader ID");
					goto OnExit;
				}
			}
			// Set the source code into the shader
			{
				const GLsizei shaderSourceCount = 1;
				const GLchar* shaderSources[shaderSourceCount] =
				{
					reinterpret_cast<const GLchar *>(shaderStr)
				};
				const GLint shaderSourceLengths[shaderSourceCount] =
				{
					static_cast<GLint>(size)
				};
				glShaderSource(shaderId, shaderSourceCount, shaderSources, NULL);
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
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
							std::stringstream errorMessage;
							errorMessage << "OpenGL failed to get compilation info of the fragment shader source code: " <<
								reinterpret_cast<const char*>(gluErrorString(errorCode));
							eae6320::UserOutput::Print(errorMessage.str());
							goto OnExit;
						}
					}
					else
					{
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
							std::stringstream errorMessage;
							errorMessage << "The fragment shader failed to compile:\n" << compilationInfo;
							eae6320::UserOutput::Print(errorMessage.str());
							goto OnExit;
						}
					}
					else
					{
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
			// free not providing a const argument is a problem with the API.
			// therefore this hard-cast is necessary.  sorry.  -bug
			free((char *)shaderStr);
			shaderStr = NULL;
		}

		return shaderId;
	}

	eae6320::Graphics::Effect::VertexShader LoadVertexShader(
		eae6320::Graphics::Effect::Parent program,
		eae6320::Graphics::Effect::CompiledShader compiledShader)
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


	eae6320::Graphics::Effect::FragmentShader LoadFragmentShader(
		eae6320::Graphics::Effect::Parent program,
		eae6320::Graphics::Effect::CompiledShader compiledShader)
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

	bool FinishUp(eae6320::Graphics::Effect * effect)
	{
		glLinkProgram(effect->parent);
		GLenum errorCode = glGetError();
		if (errorCode == GL_NO_ERROR)
		{
			// Get link info
			// (this won't be used unless linking fails
			// but it can be useful to look at when debugging)
			std::string linkInfo;
			{
				GLint infoSize;
				glGetProgramiv(effect->parent, GL_INFO_LOG_LENGTH, &infoSize);
				errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					sLogInfo info(static_cast<size_t>(infoSize));
					GLsizei* dontReturnLength = NULL;
					glGetProgramInfoLog(effect->parent, static_cast<GLsizei>(infoSize), dontReturnLength, info.memory);
					errorCode = glGetError();
					if (errorCode == GL_NO_ERROR)
					{
						linkInfo = info.memory;
					}
					else
					{
						std::stringstream errorMessage;
						errorMessage << "OpenGL failed to get link info of the program: " <<
							reinterpret_cast<const char*>(gluErrorString(errorCode));
						eae6320::UserOutput::Print(errorMessage.str());
						return false;
					}
				}
				else
				{
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to get the length of the program link info: " <<
						reinterpret_cast<const char*>(gluErrorString(errorCode));
					eae6320::UserOutput::Print(errorMessage.str());
					return false;
				}
			}
			// Check to see if there were link errors
			GLint didLinkingSucceed;
			{
				glGetProgramiv(effect->parent, GL_LINK_STATUS, &didLinkingSucceed);
				errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					if (didLinkingSucceed == GL_FALSE)
					{
						std::stringstream errorMessage;
						errorMessage << "The program failed to link:\n" << linkInfo;
						eae6320::UserOutput::Print(errorMessage.str());
						return false;
					}
				}
				else
				{
					std::stringstream errorMessage;
					errorMessage << "OpenGL failed to find out if linking of the program succeeded: " <<
						reinterpret_cast<const char*>(gluErrorString(errorCode));
					eae6320::UserOutput::Print(errorMessage.str());
					return false;
				}
			}

			GLint uniform_handle_1, uniform_handle_2, uniform_handle_3;
			{
				uniform_handle_1 = glGetUniformLocation(effect->parent, "g_local2world");
				if (uniform_handle_1 == -1)
				{
					eae6320::UserOutput::Print("No g_local2world uniform found");
					return false;
				}
				uniform_handle_2 = glGetUniformLocation(effect->parent, "g_world2view");
				if (uniform_handle_2 == -1)
				{
					eae6320::UserOutput::Print("No g_world2view uniform found");
					return false;
				}uniform_handle_3 = glGetUniformLocation(effect->parent, "g_view2screen");
				if (uniform_handle_3 == -1)
				{
					eae6320::UserOutput::Print("No g_view2screen uniform found");
					return false;
				}


			}
			effect->uni_local2world = uniform_handle_1;
			effect->uni_world2view = uniform_handle_2;
			effect->uni_view2screen = uniform_handle_3;
		}
		else
		{
			std::stringstream errorMessage;
			errorMessage << "OpenGL failed to link the program: " <<
				reinterpret_cast<const char*>(gluErrorString(errorCode));
			eae6320::UserOutput::Print(errorMessage.str());
			return false;
		}
		
		return true;
	}

#elif defined ( EAE6320_PLATFORM_D3D )

	eae6320::Graphics::Effect::Parent CreateParent()
	{
		return eae6320::Graphics::GetDevice();
	}

	std::pair<const DWORD *, ID3DXConstantTable *> CompileShader(
		eae6320::Graphics::Effect::Parent device,
		const char * shaderStr,
		size_t size,
		eae6320::Graphics::Effect::ShaderType type,
		const char *filename)
	{
		// Load the source code from file and compile it
		const DWORD * compiledShader = reinterpret_cast<const DWORD*>(shaderStr);
		ID3DXConstantTable* constants = NULL;
		D3DXGetShaderConstantTable(reinterpret_cast<const DWORD*>(shaderStr), &constants);
		return std::pair<const DWORD *, ID3DXConstantTable *>(compiledShader, constants);
	}

	std::pair<IDirect3DVertexShader9 *, ID3DXConstantTable *> LoadVertexShader(
		eae6320::Graphics::Effect::Parent device,
		std::pair<const DWORD *, ID3DXConstantTable *> compiledShader)
	{
		IDirect3DVertexShader9 * shader;
		const DWORD * buffer = compiledShader.first;
		ID3DXConstantTable * constants = compiledShader.second;
		// Create the vertex shader object
		bool wereThereErrors = false;
		{
			const HRESULT result = device->CreateVertexShader(buffer, &shader);
			if (FAILED(result))
			{
				eae6320::UserOutput::Print("Direct3D failed to create the vertex shader");
				wereThereErrors = true;
			}
		}
		return std::pair<IDirect3DVertexShader9 *, ID3DXConstantTable *>(shader, constants);
	}

	std::pair<IDirect3DPixelShader9 *, ID3DXConstantTable *> LoadFragmentShader(
		eae6320::Graphics::Effect::Parent device,
		std::pair<const DWORD *, ID3DXConstantTable *> compiledShader)
	{
		IDirect3DPixelShader9 * shader;
		const DWORD * buffer = compiledShader.first;
		ID3DXConstantTable * constants = compiledShader.second;
		// Create the vertex shader object
		bool wereThereErrors = false;
		{
			const HRESULT result = device->CreatePixelShader(buffer, &shader);
			if (FAILED(result))
			{
				eae6320::UserOutput::Print("Direct3D failed to create the fragment shader");
				wereThereErrors = true;
			}
		}
		return std::pair<IDirect3DPixelShader9 *, ID3DXConstantTable *>(shader, constants);
	}

	bool FinishUp(eae6320::Graphics::Effect * effect)
	{
		D3DXHANDLE uniform_handle_1, uniform_handle_2, uniform_handle_3;
		ID3DXConstantTable * constants = effect->vertex_shader.second;
		uniform_handle_1 = constants->GetConstantByName(NULL, "g_local2world");
		if (!uniform_handle_1)
		{
			eae6320::UserOutput::Print("No g_local2world uniform found");
			return false;
		}
		effect->uni_local2world = uniform_handle_1;
		uniform_handle_2 = constants->GetConstantByName(NULL, "g_world2view");
		if (!uniform_handle_2)
		{
			eae6320::UserOutput::Print("No g_world2view uniform found");
			return false;
		}
		effect->uni_world2view = uniform_handle_2;
		uniform_handle_3 = constants->GetConstantByName(NULL, "g_view2screen");
		if (!uniform_handle_3)
		{
			eae6320::UserOutput::Print("No g_view2screen uniform found");
			return false;
		}
		effect->uni_view2screen = uniform_handle_3;
		return true;
	}
#endif
}
