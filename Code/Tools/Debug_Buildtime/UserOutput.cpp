// Header Files
//=============

#include "UserOutput.h"
#include <iostream>

#include "../../Engine/Windows/WindowsIncludes.h"

void eae6320::UserOutput::Print( std::string output, std::string filename )
{
	std::cerr << "Error: ";
	if (!filename.empty())
		std::cerr << "(" << filename << ") ";
	std::cerr << output << "\n";
}