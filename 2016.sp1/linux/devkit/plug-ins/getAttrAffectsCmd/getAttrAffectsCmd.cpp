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

#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MItSelectionList.h>
#include <maya/MSelectionList.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MObjectArray.h>
#include <maya/MGlobal.h>
#include <maya/MSimple.h>


DeclareSimpleCommand( getAttrAffects, PLUGIN_COMPANY, "3.0");

MStatus getAttrAffects::doIt( const MArgList& args )
{
	MStatus stat;
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
	MItSelectionList iter( list );

	// Iterate over all selected dependency nodes
	//
	for ( ; !iter.isDone(); iter.next() ) 
	{
		MObject object;

		stat = iter.getDependNode( object );
		if ( !stat ) {
			stat.perror("getDependNode");
			continue;
		}

		// Create a function set for the dependency node
		//
		MFnDependencyNode node( object );

		cout << node.name() << ":\n";
		unsigned i, numAttributes = node.attributeCount();

		for (i = 0; i < numAttributes; ++i) {
			MObject attrObject = node.attribute(i);
			MFnAttribute attr;
			unsigned j, affectedLen, affectedByLen;

			attr.setObject (attrObject);

			// Get all attributes that this one affects
			MObjectArray affectedAttributes;
			node.getAffectedAttributes( attrObject, affectedAttributes );
			affectedLen = affectedAttributes.length();

			// Get all attributes that affect this one
			MObjectArray affectedByAttributes;
			node.getAffectedByAttributes( attrObject, affectedByAttributes );
			affectedByLen = affectedByAttributes.length();

			if ( affectedLen > 0 || affectedByLen > 0 ) {
				cout << "  " << attr.name() << ":\n";

				// List all attributes that are affected by the current one
				if ( affectedLen > 0 ) {
					cout << "    Affects(" << affectedLen << "):";

					for (j = 0; j < affectedLen; ++j ) {
						attr.setObject ( affectedAttributes[j] );
						cout << " " << attr.name();
					}
					cout << endl;
				}

				// List all attributes that affect the current one
				if ( affectedByLen > 0 ) {
					cout << "    AffectedBy(" << affectedByLen << "):";

					for (j = 0; j < affectedByLen; ++j ) {
						attr.setObject ( affectedByAttributes[j] );
						cout << " " << attr.name();
					}
					cout << endl;
				}
			}
		}
	}
	return MS::kSuccess;
}
