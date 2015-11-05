/*
	This file should be #included in every shader
	to set up platform-specific preprocessor directives
	so that the shader itself can be mostly platform-independent
*/

#if defined( EAE6320_PLATFORM_D3D )

// Translate GLSL variable types to HLSL
#define vec2 float2
#define vec3 float3
#define vec4 float4

#elif defined( EAE6320_PLATFORM_GL )

// The version of GLSL to use must come first
#version 330

// This extension is required in order to specify explicit locations for shader inputs and outputs
#extension GL_ARB_separate_shader_objects : require

#define o_position gl_Position
#endif
