// Header Files
//=============

#include "UserOutput.h"
#include <sstream>

#include "../../Engine/Windows/WindowsIncludes.h"

void eae6320::UserOutput::Print( std::string output, std::string filename )
{
	std::stringstream decoratedErrorMessage;
	decoratedErrorMessage << (filename.empty() ? "Asset Build" : filename) <<
		": error: " << output;
	MessageBox(NULL, decoratedErrorMessage.str().c_str(), "Error", MB_OK | MB_ICONERROR);
}