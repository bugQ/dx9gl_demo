// Header Files
//=============

#include "cEffectBuilder.h"

#include <sstream>
#include <fstream>
#include "../../Engine/Windows/WindowsFunctions.h"
#include "../Debug_Buildtime/UserOutput.h"
#include "../../Engine/Graphics/Effect.h"

// Interface
//==========

// Build
//------
using namespace eae6320::Graphics;

bool eae6320::cEffectBuilder::Build( const std::vector<std::string>& )
{
	bool wereThereErrors = false;

	// Copy the source to the target
	{
	}
	
	return !wereThereErrors;
}
