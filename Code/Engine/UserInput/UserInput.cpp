// Header Files
//=============

#include "UserInput.h"

#include "../Windows/WindowsIncludes.h"

// Helper Function Declarations
//=============================

namespace
{
	bool IsVirtualKeyPressed( const int i_virtualKeyCode );
}

// Interface
//==========

bool eae6320::UserInput::IsKeyPressed( const int i_virtualKeyCode )
{
	return IsVirtualKeyPressed( i_virtualKeyCode );
}

bool eae6320::UserInput::IsMouseButtonPressed( const int i_virtualButtonCode )
{
	return IsVirtualKeyPressed( i_virtualButtonCode );
}

// Helper Function Definitions
//============================

namespace
{
	bool IsVirtualKeyPressed( const int i_virtualKeyCode )
	{
		short keyState = GetAsyncKeyState( i_virtualKeyCode );
		const short isKeyDownMask = ~1;
		return ( keyState & isKeyDownMask ) != 0;
	}
}
