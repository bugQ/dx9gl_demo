/*
	The main() function is where the program starts execution
*/

// Header Files
//=============

#include "cEffectBuilder.h"

// Entry Point
//============

int main( int i_argumentCount, char** i_arguments )
{
	return eae6320::Build<eae6320::cEffectBuilder>( i_arguments, i_argumentCount );
}
