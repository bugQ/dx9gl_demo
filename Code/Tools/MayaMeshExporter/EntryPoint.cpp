/*
	The two functions in this file are how Maya will interact with the plug-in
*/

// Header Files
//=============

#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MStatus.h>

#include "cMayaMeshExporter.h"

// Static Data Initialization
//===========================

namespace
{
	// This will be displayed in Maya's dropdown list of available export formats
	const char* s_pluginName = "Bug's Monster Mesh";
}

// Entry Point
//============

__declspec(dllexport) MStatus initializePlugin( MObject io_object )
{
	// Create a plugin function set
	MFnPlugin plugin( io_object );

	// Register the exporter
	MStatus status;
	{
		char* noIcon = "none";
		status = plugin.registerFileTranslator( s_pluginName, noIcon,
			// This function is what Maya should call to create a new instance of the mesh exporter
			eae6320::cMayaMeshExporter::Create );
		if ( !status )
		{
			MGlobal::displayError( MString( "Failed to register mesh exporter: " ) + status.errorString() );
		}
	}
    return status;
}

__declspec(dllexport) MStatus uninitializePlugin( MObject io_object )
{
	// Create a plugin function set
	MFnPlugin plugin( io_object );

	// Register the exporter
	MStatus status;
	{
		status = plugin.deregisterFileTranslator( s_pluginName );
		if ( !status )
		{
			MGlobal::displayError( MString( "Failed to deregister mesh exporter: " ) + status.errorString() );
		}
	}
    return status;
}
