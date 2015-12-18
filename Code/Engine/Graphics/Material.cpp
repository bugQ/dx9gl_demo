#include "stdafx.h"
#include "Material.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include "../Debug_Runtime/UserOutput.h"

namespace eae6320
{
namespace Graphics
{
	bool Material::SetParams()
	{
		for (size_t i = 0; i < num_params; ++i)
			if (params[i].vec_length > 0
				&& !effect->SetVec(
					params[i].handle,
					params[i].shaderType,
					params[i].vec,
					params[i].vec_length))
				return false;

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

		for (size_t i = 0; i < num_params; ++i)
		{
			Material::UniformParameter & param = mat->params[i];
			char * param_name = scanptr + UHANDLE2DIFF(param.handle);

			Effect::UniformHandle handle = mat->effect->GetUniformHandle(param_name,
				param.shaderType);

			if (handle == INVALID_UNIFORM_HANDLE)
				goto OnError;

			mat->params[i].handle = handle;
		}

		return mat;

	OnError:
		delete mat;
		return NULL;
	}

	Material::~Material()
	{
		delete effect;
		delete[] params;
	}
}
}