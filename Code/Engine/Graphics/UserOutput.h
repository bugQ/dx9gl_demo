/*
	Definitions for console output functions
*/
#ifndef EAE6320_USEROUTPUT_H
#define EAE6320_USEROUTPUT_H

// Header Files
//=============

#include "Windows/WindowsIncludes.h"
#include <string>

// Interface
//==========

namespace eae6320
{
	namespace UserOutput
	{
		void Print( std::string output );
	}
}

#endif