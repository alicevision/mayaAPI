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

#include <maya/MSimple.h>
#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MItSelectionList.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MIOStream.h>

DeclareSimpleCommand( whatis, PLUGIN_COMPANY, "3.0");

MStatus whatis::doIt( const MArgList& args )
{

	MSelectionList list;

	if ( args.length() > 0 ) {
		// Arg list is > 0 so use objects that were passes in
		//
		MString argStr;

		unsigned last = args.length();
		for ( unsigned i = 0; i < last; i++ ) {
			// Attempt to find all of the objects matched
			// by the string and add them to the list
			//
			args.get( i, argStr );  
			list.add( argStr ); 
		}
	} else {
		// Get the geometry list from what is currently selected in the 
		// model
		//
		MGlobal::getActiveSelectionList( list );
	} 

	MObject node;
	MFnDependencyNode depFn;
	MStringArray types;
	MString name;

	for ( MItSelectionList iter( list ); !iter.isDone(); iter.next() ) {
		// Print the types and function sets of all of the objects
		//
		iter.getDependNode( node );
		depFn.setObject( node );
		
		name = depFn.name();
		MGlobal::getFunctionSetList( node, types );

		cout << "Name: " << name.asChar() << endl;
		cout << "Type: " << node.apiTypeStr() << endl;
		cout << "Function Sets: ";
		
		int last = types.length();
		for( int i = 0; i < last; i++ ) {
			if ( i > 0 ) cout << ", ";
			cout << types[i].asChar();
		}
		cout << endl << endl;
	}
	return MS::kSuccess;
}
