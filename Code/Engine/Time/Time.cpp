// Header Files
//=============

#include "Time.h"

#include <cassert>
#include "../Windows/WindowsIncludes.h"
#include "../Windows/WindowsFunctions.h"

// Static Data Initialization
//===========================

namespace
{
	bool s_isInitialized = false;

	double s_secondsPerCount = std::numeric_limits<double>::infinity();
	long long s_countsPerSecond = 0;
	LARGE_INTEGER s_totalCountsElapsed_atInitializion = { 0 };
	LARGE_INTEGER s_totalCountsElapsed_duringRun = { 0 };
	LARGE_INTEGER s_totalCountsElapsed_previousFrame = { 0 };
	long long s_totalCountsElapsed_currentSecond = 0;
	int s_totalFramesElapsed_currentSecond = 0;
	int s_measuredFramesPerSecond = 0;

}

// Helper Function Declarations
//=============================

namespace
{
	bool InitializeIfNecessary();
}

// Interface
//==========

// Time
//-----

float eae6320::Time::GetTotalSecondsElapsed()
{
	{
		const bool result = InitializeIfNecessary();
		assert( result );
	}

	return static_cast<float>( static_cast<double>( s_totalCountsElapsed_duringRun.QuadPart ) * s_secondsPerCount );
}

float eae6320::Time::GetSecondsElapsedThisFrame()
{
	{
		const bool result = InitializeIfNecessary();
		assert( result );
	}

	return static_cast<float>(
		static_cast<double>( s_totalCountsElapsed_duringRun.QuadPart - s_totalCountsElapsed_previousFrame.QuadPart )
		* s_secondsPerCount );
}

int eae6320::Time::GetFramesPerSecond()
{
	return s_measuredFramesPerSecond;
}

void eae6320::Time::OnNewFrame()
{
	{
		const bool result = InitializeIfNecessary();
		assert( result );
	}

	// Update the previous frame
	{
		s_totalCountsElapsed_previousFrame = s_totalCountsElapsed_duringRun;
	}
	// Update the current frame
	{
		LARGE_INTEGER totalCountsElapsed;
		const BOOL result = QueryPerformanceCounter( &totalCountsElapsed );
		assert( result != FALSE );
		s_totalCountsElapsed_duringRun.QuadPart = totalCountsElapsed.QuadPart - s_totalCountsElapsed_atInitializion.QuadPart;
	}
	// Update the FPS counter
	{
		s_totalCountsElapsed_currentSecond += s_totalCountsElapsed_duringRun.QuadPart - s_totalCountsElapsed_previousFrame.QuadPart;
		s_totalFramesElapsed_currentSecond += 1;
		if (s_totalCountsElapsed_currentSecond > s_countsPerSecond)
		{
			s_totalCountsElapsed_currentSecond -= s_countsPerSecond;
			s_measuredFramesPerSecond = s_totalFramesElapsed_currentSecond;
			s_totalFramesElapsed_currentSecond = 0;
		}
	}
}

// Initialization / Shut Down
//---------------------------

bool eae6320::Time::Initialize( std::string* o_errorMessage )
{
	bool wereThereErrors = false;

	// Get the frequency of the high-resolution performance counter
	{
		LARGE_INTEGER countsPerSecond;
		if ( QueryPerformanceFrequency( &countsPerSecond ) != FALSE )
		{
			if ( countsPerSecond.QuadPart != 0 )
			{
				s_countsPerSecond = countsPerSecond.QuadPart;
				s_secondsPerCount = 1.0 / static_cast<double>( s_countsPerSecond );
			}
			else
			{
				wereThereErrors = true;
				if ( o_errorMessage )
				{
					*o_errorMessage = "This hardware doesn't support high resolution performance counters!";
				}
				goto OnExit;
			}
		}
		else
		{
			wereThereErrors = true;
			if ( o_errorMessage )
			{
				*o_errorMessage = GetLastWindowsError();
			}
			goto OnExit;
		}
	}
	// Store how many counts have elapsed so far
	if ( QueryPerformanceCounter( &s_totalCountsElapsed_atInitializion ) == FALSE )
	{
		wereThereErrors = true;
		if ( o_errorMessage )
		{
			*o_errorMessage = GetLastWindowsError();
		}
		goto OnExit;
	}

	s_isInitialized = true;

OnExit:

	return !wereThereErrors;
}

// Helper Function Definitions
//============================

namespace
{
	bool InitializeIfNecessary()
	{
		if ( s_isInitialized )
		{
			return true;
		}
		else
		{
			return eae6320::Time::Initialize();
		}	
	}
}
