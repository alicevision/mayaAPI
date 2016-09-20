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

//
//  Class: latticeNoiseCmd
//
//  Description:
//      The latticeNoise command creates a new lattice (ffd) deformer.  A 
//      latticeNoise node is placed between the deformed lattice shape and the 
//      actual deformer node.  This causes the deformed object to wobble over time
//      as random continuous noise is applied to the pointes of the lattice.
//
//      Once the deformer is created, the regular 'lattice' command can be 
//      used on it to modify the standard lattice parameters.
//
//      One thing to note is that the lattice geometry displayed on the 
//      screen will not show the added noise.  This is done this way
//      so that the lattice can be modified with the usual tools without 
//      the noise node overriding all changes made to the lattice.
//
//      Also note that the noise function is reasonably computationally
//      expensive, so dense lattices will be slow to update.
//
//  eg
//     This causes the currently selected object to be deformed
//  
//       latticeNoise;
//
//     This causes the specified geometry to be deformed
//
//       latticeNoise sphereShape1; 
//

#include <maya/MIOStream.h>
#include "latticeNoise.h"
#include <maya/MObject.h>
#include <maya/MFnLatticeDeformer.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h> 
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnLattice.h>
#include <maya/MDGModifier.h>

#define McheckErr(stat,msg)         \
    if ( MS::kSuccess != stat ) {   \
        cerr << msg << endl;        \
		return MS::kFailure;        \
	}

void* latticeNoiseCmd::creator()
{
	return new latticeNoiseCmd();
}
	
MStatus latticeNoiseCmd::doIt( const MArgList& args )
{
	MStatus res = MS::kSuccess;

	MSelectionList list;

	if ( args.length() > 0 ) {
		MString argStr;

		unsigned last = args.length();
		for ( unsigned i = 0; i < last; i++ ) {
			// Get the arg as a string. 
			//
			res = args.get( i, argStr ); 
			McheckErr( res, "Invalid argument type" );

			// Attempt to find all of the objects matched
			// by the string and add them to the list
			//
			res = list.add( argStr );
			McheckErr( res, "Invalid object" );
		}
	} else {
		// Get the geometry list from what is currently selected in the 
		// model
		//
		MGlobal::getActiveSelectionList( list );
	}
	
	// Create the deformer
	//
	MFnLatticeDeformer defFn;
	MObject deformNode = defFn.create( 2, 5, 2, &res );
	McheckErr( res, "Deformer creation failed" ); 
	MObject geomObj;

	for ( MItSelectionList iter( list, MFn::kGeometric ); 
		  !iter.isDone(); iter.next() ) {
		// Get the object and add it to the deformation.  Note: we just
		// ignore objects that we don't understand.  This is standard maya
		// command behavior
		//
		iter.getDependNode( geomObj );
		defFn.addGeometry( geomObj );
	}
	
	// Reset the lattice to bound it's geometry
	//
	defFn.resetLattice( true );

	// Insert the latticeNoise node between the deformed lattice and the
	// actual lattice deformer node
	//
	MFnDependencyNode depNodeFn;
	MDGModifier modifier;
	MString attrName;

	// Make the noise node and get the attributes that we are going to 
	// connect up
	//
	MString nodeType("latticeNoise" );
	MObject noiseNode = depNodeFn.create( nodeType, &res );
	McheckErr( res, "Lattice noise node creation failed" ); 
	attrName.set( "input" );
	MObject inputAttr = depNodeFn.attribute( attrName );
	attrName.set( "output" );
	MObject outputAttr = depNodeFn.attribute( attrName );
	attrName.set( "time" );
	MObject timeAttr = depNodeFn.attribute( attrName );

	// Get the lattice input attribute from the deformer node
	//
	attrName.set( "deformedLatticePoints" );
	MObject destLattAttr = defFn.attribute( attrName );

	// Get the lattice shape node so that we can find the output 
	// attribute
	//
	MObject deformedLattice = defFn.deformLattice( &res );
	McheckErr( res, "Could not get the deformed lattice node" ); 
	MFnLattice latticeShapeFn( deformedLattice, &res );
	attrName.set( "latticeOutput" );
	MObject sourceLattAttr = latticeShapeFn.attribute( attrName );
	
	// Disconnect lattice from deformer
	//
	res = modifier.disconnect( deformedLattice, sourceLattAttr,
							   deformNode, destLattAttr );
	McheckErr( res, "Could not disconnect nodes" ); 
	
	// Insert new noise node
	//
	modifier.connect( deformedLattice, sourceLattAttr,
					  noiseNode, inputAttr );
	modifier.connect( noiseNode, outputAttr,
					  deformNode, destLattAttr );

	// Find the time node and connect to it
	//
	list.clear();
	list.add( "time1" );
	MObject timeNode;
	list.getDependNode( 0, timeNode ); 
	MFnDependencyNode timeFn( timeNode, &res );
	McheckErr( res, "Could not get the time node" );
	attrName.set( "outTime" );
	MObject timeOutAttr = timeFn.attribute( attrName );
	
	// Connect to time node
	// 
	modifier.connect( timeNode, timeOutAttr,
					  noiseNode, timeAttr );

	// Perform all of the operations cued up in the modifier
	//
	res = modifier.doIt();
	McheckErr( res, "Error changing deformer graph connections" ); 

	// Lastly, make the new lattice that we have created active
	// This is standard Mel command behaviour.
	// 
	MString name = latticeShapeFn.name();
    MGlobal::selectByName( name, MGlobal::kReplaceList );

	return res;
}
