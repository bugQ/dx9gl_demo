// Header Files
//=============

#include "stdafx.h"

#include "UserOutput.h"

#include "Windows/WindowsIncludes.h"

void eae6320::UserOutput::Print( std::string output )
{
	MessageBox(NULL, output.c_str(), "Error", MB_OK | MB_ICONERROR);
}