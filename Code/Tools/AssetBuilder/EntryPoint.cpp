/*
	The main() function is where the program starts execution
*/

// Header Files
//=============

#include <cstdlib>
#include "AssetBuilder.h"

// Entry Point
//============

int main( int i_argumentCount, char** i_arguments )
{
	bool wereThereErrors = false;

	if ( !eae6320::AssetBuilder::Initialize() )
	{
		wereThereErrors = true;
		goto OnExit;
	}

	// The command line should have a list of assets to build
	for ( int i = 1; i < i_argumentCount; ++i )
	{
		const char* const relativePath = i_arguments[i];
		if ( !eae6320::AssetBuilder::BuildAsset( relativePath ) )
		{
			wereThereErrors = true;
			// Instead of exiting immediately any assets that can be built should be built
			// (both because it gives a better idea of the number of errors
			// but also because it could potentially allow the game to still be run with the
			// assets that _did_ build successfully)
		}
	}

OnExit:

	if ( !eae6320::AssetBuilder::ShutDown() )
	{
		wereThereErrors = true;
	}

	if ( !wereThereErrors )
	{
		return EXIT_SUCCESS;
	}
	else
	{
		return EXIT_FAILURE;
	}
}
