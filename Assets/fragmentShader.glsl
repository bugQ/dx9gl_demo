/*
	This is an example of a fragment shader
*/

// The version of GLSL to use must come first
#version 330

// This extension is required in order to specify explicit locations for shader inputs and outputs
#extension GL_ARB_separate_shader_objects : require

// Input
//======

// None; the only data that the vertex shader output was gl_Position,
// and that is used by the GPU

// Output
//=======

// Whatever color value is output from the fragment shader
// will determine the color of the corresponding pixel on the screen
out vec4 o_color;

// Entry Point
//============

void main()
{
	// For now set the fragment to white
	// (where color is represented by 4 floats representing "RGBA" == "Red/Green/Blue/Alpha").
	// If you are curious you should experiment with changing the values of the first three numbers
	// to something in the range [0,1] and observing the results
	// (although when you submit your Assignment 01 the color output must be white).
	o_color = vec4( 1.0, 1.0, 1.0, 1.0 );
}
