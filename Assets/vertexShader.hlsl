/*
	This is an example of a vertex shader
*/

// Entry Point
//============

void main(

	// Input
	//======

	// The "semantics" (the keywords in all caps after the colon) are arbitrary,
	// but must match the C call to CreateVertexDeclaration()

	// This value comes from one of the sVertex that we filled the vertex buffer with in C code
	in const float2 i_position : POSITION,

	// Output
	//=======

	// A POSITION value must always be output from every vertex shader
	// so that the GPU can figure out which fragments need to be shaded
	out float4 o_position : POSITION

	)
{
	// Calculate the position of this vertex on screen
	{
		// When we move to 3D graphics the screen position that the vertex shader outputs
		// will be different than the position that is input to it from C code,
		// but for now the "out" position is set directly from the "in" position:
		o_position = float4( i_position.x, i_position.y, 0.0, 1.0 );
		// Or, equivalently:
		o_position = float4( i_position.xy, 0.0, 1.0 );
		o_position = float4( i_position, 0.0, 1.0 );
	}
}
