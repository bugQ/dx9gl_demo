/*
	This is an example of a vertex shader
*/

// The version of GLSL to use must come first
#version 330

// This extension is required in order to specify explicit locations for shader inputs and outputs
#extension GL_ARB_separate_shader_objects : require

// Input
//======

// The locations assigned are arbitrary
// but must match the C calls to glVertexAttribPointer()

// This value comes from one of the sVertex that we filled the vertex buffer with in C code
layout( location = 0 ) in vec2 i_position;

// Output
//=======

// None; the vertex shader must always output a position value,
// but unlike HLSL where the value is explicit
// GLSL has an implicit required variable called "gl_Position"

// Entry Point
//============

void main()
{
	// Calculate position
	{
		// When we move to 3D graphics the screen position that the vertex shader outputs
		// will be different than the position that is input to it from C code,
		// but for now the "out" position is set directly from the "in" position:
		gl_Position = vec4( i_position.x, i_position.y, 0.0, 1.0 );
		// Or, equivalently:
		gl_Position = vec4( i_position.xy, 0.0, 1.0 );
		gl_Position = vec4( i_position, 0.0, 1.0 );
	}
}
