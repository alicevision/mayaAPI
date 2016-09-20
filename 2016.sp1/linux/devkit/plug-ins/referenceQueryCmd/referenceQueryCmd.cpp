//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

#include <maya/MString.h>
#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MFileIO.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>

#include <maya/MIOStream.h>

class referenceQuery : public MPxCommand { 
public:
						referenceQuery();
	virtual				~referenceQuery();

	MStatus				doIt( const MArgList & );
	bool				isUndoable() const;

	static		void   *creator();
};


referenceQuery::referenceQuery()
{
}

referenceQuery::~referenceQuery()
{
}

void *referenceQuery::creator()
{
	return new referenceQuery;
}

bool referenceQuery::isUndoable() const {
	return false;
};

MStatus referenceQuery::doIt( const MArgList &  )
{
	MStringArray referenceFiles;
	MFileIO::getReferences( referenceFiles );

	// Print some useful information about the files referenced
	// in the main scene.
	//
	// Output format is as follows:
	// 
	// Referenced File: filename1
	// 		Connections Made
	// 			source -> destination
	// 			...
	// 
	// 		Connections Broken
	//			source ->destination
	// 			...
	//
	//		Attributes Changed Since File Referenced
	//			attribute1
	//			attribute2
	//			...
	//
	for( unsigned i = 0; i < referenceFiles.length(); i++ ) {
		MStringArray  	connectionsMade;
		MFileIO::getReferenceConnectionsMade( referenceFiles[i],
											  connectionsMade );

		cout << "Referenced File: " << referenceFiles[i].asChar() << ":\n";
		cout << "	Connections Made:\n";
		unsigned j;
		for( j = 0; j < connectionsMade.length(); j+=2 ) {
			cout << "	";
			cout << connectionsMade[j].asChar() << " -> ";
			if( j + 1 < connectionsMade.length() ) {
				cout << connectionsMade[j+1].asChar();
			}
			cout << "\n" ;
		}
		cout << "\n";

		MStringArray  	connectionsBroken;
		MFileIO::getReferenceConnectionsBroken( referenceFiles[i],
											  connectionsBroken );
		cout << "	Connections Broken: \n";
		for( j = 0; j < connectionsBroken.length(); j+=2 ) {
			cout << "	";
			cout << connectionsBroken[j].asChar() << " -> ";
			if( j + 1 < connectionsBroken.length() ) {
				cout << connectionsBroken[j+1].asChar();
			}
			cout << "\n" ;
		}
		cout << "\n";

		MStringArray	referencedNodes;
		
		cout << "	Attrs Changed Since File Open:\n";
		MFileIO::getReferenceNodes( referenceFiles[i], referencedNodes );
		for( j = 0; j < referencedNodes.length(); j++ ) {
			// For each node, call a MEL command to get its
			// attributes.  Say we're only interested in scalars.
			//
			MString cmd( "listAttr -s -cfo " );
			cmd = cmd + referencedNodes[j];
			
			MSelectionList referencedPlugs;
			MStringArray   referencedAttributes;	

			MGlobal::executeCommand( cmd, referencedAttributes );
			for( unsigned k = 0; k < referencedAttributes.length(); k++ ) {
				MString plugName( referencedNodes[j] );
				plugName = plugName + "." + referencedAttributes[k];
				cout << "		" << plugName.asChar() << "\n";
			}
		}
		cout << "\n";
	}

	// End of output 
	//
	cout << "=====================================\n";

	return MS::kSuccess;
}

MStatus initializePlugin( MObject obj )
{
	MStatus   status;
    MFnPlugin plugin( obj, PLUGIN_COMPANY, "4.0", "Any");

    // NOTE: referenceQuery is already a Maya cmd
    status = plugin.registerCommand( "refQuery", 
									 referenceQuery::creator );
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

    return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus   status;
    MFnPlugin plugin( obj );

    status = plugin.deregisterCommand( "refQuery" );
	if (!status) {
		status.perror("deregisterCommand");
	}
    return status;
}
