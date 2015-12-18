#include "stdafx.h"
#include "Material.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include <assert.h>
#include <algorithm>
#include "../Debug_Runtime/UserOutput.h"
#include "../Windows/WindowsFunctions.h"

namespace
{
	eae6320::Graphics::Material::TextureHandle LoadTexture(
		const char * texture_path,
		eae6320::Graphics::Effect::Parent parent = 0);
	eae6320::Graphics::Material::Sampler GetSampler(
		eae6320::Graphics::Material & material,
		eae6320::Graphics::Effect::UniformHandle handle);
}

namespace eae6320
{
namespace Graphics
{
	bool Material::SetParams()
	{
		for (size_t i = 0; i < num_params; ++i) {
			if (params[i].vec_length == 0)
			{
				Sampler samp = *reinterpret_cast<Sampler *>(&params[i].handle);
				TextureHandle tex = *reinterpret_cast<TextureHandle *>(params[i].vec);
				TextureUnit unit = *reinterpret_cast<TextureUnit *>(params[i].vec + 2);

				SetTexture(samp, tex, unit);
			}
			else if (!effect->SetVec(
					params[i].handle,
					params[i].shaderType,
					params[i].vec,
					params[i].vec_length))
				return false;
		}

		return true;
	}

	Material * Material::FromFile(const char * materialPath, Effect::Parent parent)
	{
		HANDLE infile = CreateFile(materialPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
		FILE_STANDARD_INFO info;

		if (infile == INVALID_HANDLE_VALUE)
		{
			std::stringstream errstr;
			errstr << "Could not open path " << materialPath << "\n";
			UserOutput::Print(errstr.str(), __FILE__);
			return NULL;
		}

		if (!GetFileInformationByHandleEx(infile, FileStandardInfo, &info, sizeof(info)))
		{
			std::stringstream errstr;
			errstr << "Failed to get file info for " << materialPath << ": "
				<< GetLastError() << "\n";
			UserOutput::Print(errstr.str(), __FILE__);
			return NULL;
		}

		DWORD total_bytes = static_cast<DWORD>(info.EndOfFile.QuadPart);
		DWORD bytes_read;

		char * buf = new char[total_bytes];

		if (!ReadFile(infile, buf, total_bytes, &bytes_read, NULL)
			|| bytes_read != total_bytes)
		{
			std::stringstream errstr;
			errstr << "Failed to read " << materialPath << ": "
				<< GetLastError() << "\n";
			UserOutput::Print(errstr.str(), __FILE__);
			return NULL;
		}

		Material * mat = new Material();
		char * scanptr = buf;

		uint16_t effect_path_len = *reinterpret_cast<uint16_t *>(scanptr);
		scanptr += sizeof(effect_path_len);

		uint16_t num_params = *reinterpret_cast<uint16_t *>(scanptr);
		mat->num_params = num_params;
		scanptr += sizeof(num_params);

		
		char * effect_path = scanptr;
		mat->effect = Effect::FromFile(effect_path, parent);
		scanptr += effect_path_len;

		if (!mat->effect)
			goto OnError;

		mat->params = new Material::UniformParameter[num_params];
		memcpy(mat->params, scanptr, num_params * sizeof(Material::UniformParameter));
		scanptr += num_params * sizeof(Material::UniformParameter);

		unsigned int texture_unit = 0;
		for (size_t i = 0; i < num_params; ++i)
		{
			Material::UniformParameter & param = mat->params[i];
			char * param_name = scanptr + UHANDLE2DIFF(param.handle);

			Effect::UniformHandle handle = mat->effect->GetUniformHandle(param_name,
				param.shaderType);

			if (handle == INVALID_UNIFORM_HANDLE)
				goto OnError;


			if (mat->params[i].vec_length == 0)
			{
				char * texture_path = param_name + strlen(param_name) + 1;

				// this is horrendous and I know it
				// but it easily allows for multiple samplers
				*reinterpret_cast<TextureHandle *>(mat->params[i].vec) = LoadTexture(texture_path, parent);
				*reinterpret_cast<TextureUnit *>(mat->params[i].vec + 2) = texture_unit++;
				*reinterpret_cast<Sampler *>(&mat->params[i].handle) = GetSampler(*mat, handle);
			}
			else
				mat->params[i].handle = handle;
		}

		return mat;

	OnError:
		delete mat;
		return NULL;
	}

	bool eae6320::Graphics::Material::SetTexture(Sampler samp, TextureHandle tex, TextureUnit unit)
	{
#if defined( EAE6320_PLATFORM_GL )
		GLenum error;

		glActiveTexture(GL_TEXTURE0 + unit);
		error = glGetError();
		assert(error == GL_NO_ERROR);
		if (error != GL_NO_ERROR)
			return false;

		glBindTexture(GL_TEXTURE_2D, tex);
		error = glGetError();
		assert(error == GL_NO_ERROR);
		if (error != GL_NO_ERROR)
			return false;

		glUniform1i(samp, unit);
		error = glGetError();
		assert(error == GL_NO_ERROR);
		if (error != GL_NO_ERROR)
			return false;
#elif defined( EAE6320_PLATFORM_D3D )
		HRESULT result = this->effect->parent->SetTexture(samp, tex);
		assert(SUCCEEDED(result));
		if (!SUCCEEDED(result))
			return false;
#endif
		return true;
	}

	Material::~Material()
	{
		delete effect;
		delete[] params;
	}
}
}

namespace
{
#if defined( EAE6320_PLATFORM_GL )
	GLuint LoadTexture(
		const char * texture_path,
		GLuint parent)
	{
		GLuint o_texture;
		std::string o_errorMessage;

		bool wereThereErrors = false;
		HANDLE fileHandle = INVALID_HANDLE_VALUE;
		void* fileContents = NULL;
		o_texture = 0;

		// Open the texture file
		{
			const DWORD desiredAccess = FILE_GENERIC_READ;
			const DWORD otherProgramsCanStillReadTheFile = FILE_SHARE_READ;
			SECURITY_ATTRIBUTES* useDefaultSecurity = NULL;
			const DWORD onlySucceedIfFileExists = OPEN_EXISTING;
			const DWORD useDefaultAttributes = FILE_ATTRIBUTE_NORMAL;
			const HANDLE dontUseTemplateFile = NULL;
			fileHandle = CreateFile(texture_path, desiredAccess, otherProgramsCanStillReadTheFile,
				useDefaultSecurity, onlySucceedIfFileExists, useDefaultAttributes, dontUseTemplateFile);
			if (fileHandle == INVALID_HANDLE_VALUE)
			{
				wereThereErrors = true;
				std::string windowsErrorMessage(eae6320::GetLastWindowsError());
				std::stringstream ss;
				ss << "Windows failed to open the texture file: " << windowsErrorMessage;
				eae6320::UserOutput::Print(ss.str().c_str(), __FILE__);
				goto OnExit;
			}
		}
		// Get the file's size
		size_t fileSize;
		{
			LARGE_INTEGER fileSize_integer;
			if (GetFileSizeEx(fileHandle, &fileSize_integer) != FALSE)
			{
				assert(fileSize_integer.QuadPart <= SIZE_MAX);
				fileSize = static_cast<size_t>(fileSize_integer.QuadPart);
			}
			else
			{
				wereThereErrors = true;
				std::string windowsErrorMessage(eae6320::GetLastWindowsError());
				std::stringstream ss;
				ss << "Windows failed to get the size of the texture file: " << windowsErrorMessage;
				eae6320::UserOutput::Print(ss.str().c_str(), __FILE__);
				goto OnExit;
			}
		}
		// Read the file's contents into temporary memory
		fileContents = malloc(fileSize);
		if (fileContents)
		{
			DWORD bytesReadCount;
			OVERLAPPED* readSynchronously = NULL;
			assert(fileSize < (uint64_t(1) << (sizeof(DWORD) * 8)));
			if (ReadFile(fileHandle, fileContents, static_cast<DWORD>(fileSize),
				&bytesReadCount, readSynchronously) == FALSE)
			{
				wereThereErrors = true;
				std::string windowsErrorMessage(eae6320::GetLastWindowsError());
				std::stringstream ss;
				ss << "Windows failed to read the contents of the texture file: " << windowsErrorMessage;
				eae6320::UserOutput::Print(ss.str().c_str(), __FILE__);
				goto OnExit;
			}
		}
		else
		{
			wereThereErrors = true;
			std::stringstream ss;
			ss << "Failed to allocate " << fileSize << " bytes to read in the texture " << texture_path;
			eae6320::UserOutput::Print(ss.str().c_str(), __FILE__);
			goto OnExit;
		}

		// Create a new texture and make it active
		{
			const GLsizei textureCount = 1;
			glGenTextures(textureCount, &o_texture);
			const GLenum errorCode = glGetError();
			if (errorCode == GL_NO_ERROR)
			{
				// This code only supports 2D textures;
				// if you want to support other types you will need to improve this code.
				glBindTexture(GL_TEXTURE_2D, o_texture);
				const GLenum errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					wereThereErrors = true;
					std::stringstream ss;
					ss << "OpenGL failed to bind a new texture: " <<
						reinterpret_cast<const char*>(gluErrorString(errorCode));
					eae6320::UserOutput::Print(ss.str().c_str(), __FILE__);
					goto OnExit;
				}
			}
			else
			{
				wereThereErrors = true;
				std::stringstream ss;
				ss << "OpenGL failed to get an unused texture ID: " <<
					reinterpret_cast<const char*>(gluErrorString(errorCode));
				eae6320::UserOutput::Print(ss.str().c_str(), __FILE__);
				goto OnExit;
			}
		}

		// Extract the data
		const uint8_t* currentPosition = reinterpret_cast<uint8_t*>(fileContents);
		// Verify that the file is a valid DDS
		{
			const size_t fourCcCount = 4;
			const uint8_t* const fourCc = currentPosition;
			const uint8_t fourCc_dds[fourCcCount] = { 'D', 'D', 'S', ' ' };
			// Each of the four characters can be compared in a single operation by casting to a uint32_t
			const bool isDds = *reinterpret_cast<const uint32_t*>(fourCc) == *reinterpret_cast<const uint32_t*>(fourCc_dds);
			if (isDds)
			{
				currentPosition += fourCcCount;
			}
			else
			{
				wereThereErrors = true;
				char fourCcString[fourCcCount + 1] = { 0 };	// Add NULL terminator
				memcpy(fourCcString, currentPosition, fourCcCount);
				std::stringstream ss;
				ss << "The texture file \"" << texture_path << "\" isn't a valid DDS. The Four CC is \"" << fourCcString << "\""
					" instead of \"DDS \"";
				eae6320::UserOutput::Print(ss.str().c_str(), __FILE__);
				goto OnExit;
			}
		}
		// Extract the header
		// (this struct can also be found in Dds.h in the DirectX header files
		// or here as of this comment: https://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx )
		struct sDdsHeader
		{
			uint32_t structSize;
			uint32_t flags;
			uint32_t height;
			uint32_t width;
			uint32_t pitchOrLinearSize;
			uint32_t depth;
			uint32_t mipMapCount;
			uint32_t reserved1[11];
			struct
			{
				uint32_t structSize;
				uint32_t flags;
				uint8_t fourCc[4];
				uint32_t rgbBitCount;
				uint32_t bitMask_red;
				uint32_t bitMask_green;
				uint32_t bitMask_blue;
				uint32_t bitMask_alpha;
			} pixelFormat;
			uint32_t caps[4];
			uint32_t reserved2;
		};
		const sDdsHeader* ddsHeader = reinterpret_cast<const sDdsHeader*>(currentPosition);
		currentPosition += sizeof(sDdsHeader);
		// Convert the DDS format into an OpenGL format
		GLenum format;
		{
			// This code can only handle the two basic formats that the example TextureBuilder will create.
			// If a DDS in a different format is provided to TextureBuilder it will be passed through unchanged
			// and this code won't work.
			// Similarly, if you improve the example TextureBuilder to support more formats
			// you will also have to update this code to support them.
			const uint8_t fourCc_dxt1[] = { 'D', 'X', 'T', '1' };	// No alpha channel
			const uint8_t fourCc_dxt5[] = { 'D', 'X', 'T', '5' };	// Alpha channel
			const uint32_t fourCc_texture = *reinterpret_cast<const uint32_t*>(ddsHeader->pixelFormat.fourCc);
			if (fourCc_texture == *reinterpret_cast<const uint32_t*>(fourCc_dxt1))
			{
				format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			}
			else if (fourCc_texture == *reinterpret_cast<const uint32_t*>(fourCc_dxt5))
			{
				format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			}
			else
			{
				wereThereErrors = true;
				char fourCcString[5] = { 0 };	// Add NULL terminator
				memcpy(fourCcString, ddsHeader->pixelFormat.fourCc, 4);
				std::stringstream ss;
				ss << "The texture file \"" << texture_path << "\" has an unsupported format \"" << fourCcString << "\"";
				eae6320::UserOutput::Print(ss.str().c_str(), __FILE__);
				goto OnExit;
			}
		}
		// Iterate through each MIP map level and fill in the OpenGL texture
		{
			GLsizei currentWidth = ddsHeader->width;
			GLsizei currentHeight = ddsHeader->height;
			const GLsizei blockSize = format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ? 8 : 16;
			const GLint borderWidth = 0;
			for (uint32_t mipMapLevel = 0; mipMapLevel < ddsHeader->mipMapCount; ++mipMapLevel)
			{
				const GLsizei mipMapSize = ((currentWidth + 3) / 4) * ((currentHeight + 3) / 4) * blockSize;
				glCompressedTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(mipMapLevel), format, currentWidth, currentHeight,
					borderWidth, mipMapSize, currentPosition);
				const GLenum errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					currentPosition += static_cast<size_t>(mipMapSize);
					currentWidth = std::max(1, (currentWidth / 2));
					currentHeight = std::max(1, (currentHeight / 2));
				}
				else
				{
					wereThereErrors = true;
					std::stringstream ss;
					ss << "OpenGL rejected compressed texture data: " <<
						reinterpret_cast<const char*>(gluErrorString(errorCode));
					eae6320::UserOutput::Print(ss.str().c_str(), __FILE__);
					
					goto OnExit;
				}
			}
		}

		assert(currentPosition == (reinterpret_cast<uint8_t*>(fileContents) + fileSize));

	OnExit:

		if (fileContents != NULL)
		{
			free(fileContents);
			fileContents = NULL;
		}
		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			if (CloseHandle(fileHandle) == FALSE)
			{
				wereThereErrors = true;
				std::string windowsErrorMessage(eae6320::GetLastWindowsError());
				std::stringstream ss;
				ss << "\nWindows failed to close the texture file handle: " << windowsErrorMessage;
				eae6320::UserOutput::Print(ss.str().c_str(), __FILE__);
			}
			fileHandle = INVALID_HANDLE_VALUE;
		}
		if (wereThereErrors && (o_texture != 0))
		{
			const GLsizei textureCount = 1;
			glDeleteTextures(textureCount, &o_texture);
			assert(glGetError == GL_NO_ERROR);
			o_texture = 0;
		}

		return o_texture;
	}

	GLint GetSampler(eae6320::Graphics::Material & material, GLint handle)
	{
		return handle;
	}

#elif defined ( EAE6320_PLATFORM_D3D )
	LPDIRECT3DTEXTURE9 LoadTexture(
		const char * texture_path,
		LPDIRECT3DDEVICE9 parent)
	{
		LPDIRECT3DTEXTURE9 handle;

		const unsigned int useDimensionsFromFile = D3DX_DEFAULT_NONPOW2;
		const unsigned int useMipMapsFromFile = D3DX_FROM_FILE;
		const DWORD staticTexture = 0;
		const D3DFORMAT useFormatFromFile = D3DFMT_FROM_FILE;
		const D3DPOOL letD3dManageMemory = D3DPOOL_MANAGED;
		const DWORD useDefaultFiltering = D3DX_DEFAULT;
		const D3DCOLOR noColorKey = 0;
		D3DXIMAGE_INFO* noSourceInfo = NULL;
		PALETTEENTRY* noColorPalette = NULL;
		const HRESULT result = D3DXCreateTextureFromFileEx(parent, texture_path, useDimensionsFromFile, useDimensionsFromFile, useMipMapsFromFile,
			staticTexture, useFormatFromFile, letD3dManageMemory, useDefaultFiltering, useDefaultFiltering, noColorKey, noSourceInfo, noColorPalette,
			&handle);

		assert(SUCCEEDED(result));
		
		return handle;
	}

	DWORD GetSampler(eae6320::Graphics::Material & material, D3DXHANDLE handle)
	{
		return material.effect->fragment_shader.second->GetSamplerIndex(handle);
	}
#endif
}