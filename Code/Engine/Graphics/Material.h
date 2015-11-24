#pragma once

#include "Effect.h"
#include <vector>

namespace eae6320
{
namespace Graphics
{
	struct Material
	{

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

		~Material();
	};
}
}
