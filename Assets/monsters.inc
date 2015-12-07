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
#define mat2 float2x2
#define mat3 float3x3
#define mat4 float4x4

#define mainh_v_BEGIN void main(
#define mainh_f_BEGIN void main(
#define mainh_i_arg(n, name, datatype, semantic)  in const datatype name : semantic,
#define mainh_o_arg(n, name, datatype, semantic)  out datatype name : semantic,
#define mainh_v_END out float4 o_position : POSITION)
#define mainh_f_END out float4 o_color : COLOR0)

#elif defined( EAE6320_PLATFORM_GL )

// The version of GLSL to use must come first
#version 330

// This extension is required in order to specify explicit locations for shader inputs and outputs
#extension GL_ARB_separate_shader_objects : require

#define mainh_v_BEGIN 
#define mainh_f_BEGIN 
#define mainh_i_arg(n, name, datatype, semantic) layout( location = n )  in datatype name;
#define mainh_o_arg(n, name, datatype, semantic) layout( location = n )  out datatype name;
#define mainh_v_END void main()
#define mainh_f_END out vec4 o_color;\
void main()

#define o_position gl_Position

#define tex2D texture2D

#define mul(a, b) ((a)*(b))

#endif

#define mainh_i_POSITION(n, name) mainh_i_arg(n, name, vec3, POSITION)
#define mainh_i_COLOR(n, name) mainh_i_arg(n, name, vec4, COLOR)
#define mainh_i_COLOR0(n, name) mainh_i_arg(n, name, vec4, COLOR0)
#define mainh_i_UV(n, name) mainh_i_arg(n, name, vec2, TEXCOORD0)
#define mainh_o_POSITION(n, name) mainh_o_arg(n, name, vec4, POSITION)
#define mainh_o_COLOR(n, name) mainh_o_arg(n, name, vec4, COLOR)
#define mainh_o_COLOR0(n, name) mainh_o_arg(n, name, vec4, COLOR0)
#define mainh_o_UV(n, name) mainh_o_arg(n, name, vec2, TEXCOORD0)
