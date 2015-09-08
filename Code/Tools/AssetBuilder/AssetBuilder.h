/*
	These functions are responsible for building assets
*/

#ifndef EAE6320_ASSETBUILDER_HELPERFUNCTIONS_H
#define EAE6320_ASSETBUILDER_HELPERFUNCTIONS_H

// Interface
//==========

namespace eae6320
{
	namespace AssetBuilder
	{
		bool BuildAsset( const char* const i_relativePath );

		// Initialization / Shutdown
		//--------------------------

		bool Initialize();
		bool ShutDown();
	}
}

#endif	// EAE6320_ASSETBUILDER_HELPERFUNCTIONS_H
