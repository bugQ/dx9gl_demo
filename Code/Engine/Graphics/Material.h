#pragma once

#include "Effect.h"
#include <vector>

namespace eae6320
{
namespace Graphics
{
	struct Material
	{
		typedef
#if defined( EAE6320_PLATFORM_GL )
			GLuint
#elif defined( EAE6320_PLATFORM_D3D )
			LPDIRECT3DTEXTURE9
#endif
			TextureHandle;

		typedef
#if defined( EAE6320_PLATFORM_GL )
			GLint
#elif defined( EAE6320_PLATFORM_D3D )
			DWORD
#endif
			Sampler;

		typedef
			unsigned int
			TextureUnit;

		struct UniformParameter
		{
			Effect::UniformHandle handle;
			Effect::ShaderType shaderType;
			float vec[4];
			uint8_t vec_length;
		};

		struct Spec
		{
			std::string effect_path;
			uint16_t num_params;
			std::vector<UniformParameter> params;
			std::vector<std::string> param_names;
		};

		Effect * effect;
		UniformParameter * params;
		uint16_t num_params;

		static Material * FromFile(const char * materialPath, Effect::Parent parent = 0);
		bool SetParams();
		bool SetTexture(Sampler samp, TextureHandle tex, TextureUnit unit);

		~Material();
	};
}
}
