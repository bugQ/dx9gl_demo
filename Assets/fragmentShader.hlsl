/*
	This is an example of a fragment shader
*/

// Entry Point
//============

void main(

	// Input
	//======

	// None; the only data that the vertex shader output was POSITION,
	// and that is used by the GPU

	// Output
	//=======

	// Whatever color value is output from the fragment shader
	// will determine the color of the corresponding pixel on the screen
	out float4 o_color : COLOR0

	)
{
	// For now set the fragment to white
	// (where color is represented by 4 floats representing "RGBA" == "Red/Green/Blue/Alpha").
	// If you are curious you should experiment with changing the values of the first three numbers
	// to something in the range [0,1] and observing the results
	// (although when you submit your Assignment 01 the color output must be white).
	o_color = float4( 1.0, 1.0, 1.0, 1.0 );
}
