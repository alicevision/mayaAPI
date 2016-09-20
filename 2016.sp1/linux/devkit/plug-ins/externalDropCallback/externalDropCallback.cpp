//-
// Copyright (C) 2010 Autodesk, Inc.
//
// These coded instructions, statements and computer programs contain
// unpublished information proprietary to Autodesk, Inc. and
// are protected by the Canadian and US federal copyright law. They
// may not be disclosed to third parties or copied or duplicated, in
// whole or in part, without prior written consent of
// Autodesk Inc.
//
// Unpublished-rights reserved under the Copyright Laws of
// the United States.
//+

#include <maya/MIOStream.h>

#include <maya/MExternalDropCallback.h> 
#include <maya/MExternalDropData.h> 
#include <maya/MFnPlugin.h> 
#include <maya/MGlobal.h> 
#include <maya/MStringArray.h> 

class ExampleExternalDropCallback : public MExternalDropCallback
{
	public:
		MExternalDropStatus	externalDropCallback( bool doDrop, const MString& controlName, const MExternalDropData& data )
		{
			// dump some data to the output window
			//
			std::cout << "externalDropCallback: " << doDrop << ", \"" << controlName << "\"" << std::endl;
			std::cout << "  hasText(): " << data.hasText() << ", \"" << data.text() << "\"" << std::endl;
			MStringArray urls = data.urls();
			std::cout << "  hasUrls(): " << data.hasUrls();
			for ( unsigned int i=0; i<urls.length(); ++i )
				std::cout << ", \"" << urls[i] << "\"";
			std::cout << std::endl;
			MStringArray formats = data.formats();
			std::cout << "  formats(): ";
			for ( unsigned int i=0; i<formats.length(); ++i )
				std::cout << ", \"" << formats[i] << "\"";
			std::cout << std::endl;

#ifdef MAYA_WANT_SHELF_EDITS
			// if the drop has a URL defining a file, and we're dropping onto a shelf,
			// create a new shelf item to open that file
			//
			if ( data.hasUrls() )
			{
				MString cmd;
				int result;
				cmd = "shelfLayout -exists \"" + controlName + "\"";
				MGlobal::executeCommand( cmd, result );
				if ( result )
				{
					MString url = data.urls()[0];
					if ( url.substring( 0, 7 ) == "file:///" )
					{
						if ( doDrop )
						{
							url = url.substring( 8, url.length()-1 );
							MStringArray bits;
							url.split( '/', bits );
							MString name = bits[ bits.length()-1 ];
							cmd = "scriptToShelf( \"" + name + "\", ";
							cmd += "\"file -open \\\"" + url + "\\\"\"";
							cmd += ", true )";
							MGlobal::executeCommand( cmd );
						}
						return kNoMayaDefaultAndAccept;
					}
				}
			}
#endif

			return kMayaDefault;
		}
};

static ExampleExternalDropCallback* theCallback = NULL;

MStatus initializePlugin(MObject obj)
{
	MStatus   status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	// create an instance of our callback object
	//
	theCallback = new ExampleExternalDropCallback;

	// add the callback to the system
	//
	MExternalDropCallback::addCallback( theCallback );

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus   status;
	MFnPlugin plugin(obj);

	// remove the callback from the system
	//
	MExternalDropCallback::removeCallback( theCallback );

	// delete the callback object
	//
	delete theCallback;
	theCallback = NULL;

	return status;
}
