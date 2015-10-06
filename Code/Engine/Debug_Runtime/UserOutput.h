/*
	Definitions for console output functions
*/
#ifndef EAE6320_USEROUTPUT_H
#define EAE6320_USEROUTPUT_H

// Header Files
//=============

#include <string>

// Interface
//==========

namespace eae6320
{
	namespace UserOutput
	{
		void Print( std::string output, std::string filename = "" );
	}
}

#endif