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

//	Description:
//
//		Traces the position of an animated object
//		and create a curve showing the object's path.
//
//	Usage:
//
//		Animate an object.
//		Select the object.
//		Run 'motionTrace;' in the command window.
//		See the object's path drawn as a curve.
//
//	Options:
//
//		-s <frame>		The start frame.  Default to 1.
//		-e <frame>		The end frame.	Default to 60.
//		-b <frame>		The by frame.  Default to 1.
//
//	See also:
//
//		node_info.cc	for how to get object attributes
//		helix.cc		for how to create a curve

#include <maya/MIOStream.h>

#include <maya/MFnPlugin.h>
#include <maya/MString.h>
#include <maya/MArgList.h>

#include <maya/MPxCommand.h>

#include <maya/MGlobal.h>
#include <maya/MTime.h>
#include <maya/MDagPath.h>
#include <maya/MItSelectionList.h>
#include <maya/MSelectionList.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MPlugArray.h>
#include <maya/MPlug.h>

#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MObjectArray.h>
#include <maya/MFnNurbsCurve.h>

///////////////////////////////////////////////////
//
// Command class declaration
//
///////////////////////////////////////////////////
class motionTrace : public MPxCommand
{
public:
					motionTrace();
	virtual			~motionTrace();

	MStatus			doIt( const MArgList& args );
	MStatus			redoIt();

	static void*	creator();

private:
	void		printType( MObject node, MString & prefix );

	double			start, end, by;	// frame range
};



///////////////////////////////////////////////////
//
// Command class implementation
//
///////////////////////////////////////////////////

motionTrace::motionTrace() {}

void* motionTrace::creator()
{
	return new motionTrace();
}

motionTrace::~motionTrace()
{
}

MStatus motionTrace::doIt( const MArgList& args )
//
// Description
//     This method is called from MEL when this command is called.
//     It should set up any class data necessary for redo/undo,
//     parse any given arguments, and then call redoIt.
//
{
	start = 1.0;
	end = 60.0;
	by = 1.0;

	MStatus stat;
	double tmp;
	unsigned i;
    // Parse the arguments.
    for ( i = 0; i < args.length(); i++ )
	{
		if ( MString( "-s" ) == args.asString( i, &stat ) &&
			 MS::kSuccess == stat)
		{
			tmp = args.asDouble( ++i, &stat );
			if ( MS::kSuccess == stat )
			start = tmp;
		}
		else if ( MString( "-e" ) == args.asString( i, &stat ) &&
				  MS::kSuccess == stat)
		{
			tmp = args.asDouble( ++i, &stat );
			if ( MS::kSuccess == stat )
			end = tmp;
		}
		else if ( MString( "-b" ) == args.asString( i, &stat ) &&
				  MS::kSuccess == stat)
		{
			tmp = args.asDouble( ++i, &stat );
			if ( MS::kSuccess == stat )
			by = tmp;
		}
	}

	stat = redoIt();

	return stat;
}

/*
-----------------------------------------

	Make a degree 1 curve from the given CVs.

-----------------------------------------
*/
static void jMakeCurve( MPointArray cvs )
{
	MStatus stat;
	unsigned int deg = 1;
	MDoubleArray knots;

	unsigned int i;
	for ( i = 0; i < cvs.length(); i++ )
		knots.append( (double) i );

    // Now create the curve
    //
    MFnNurbsCurve curveFn;

    curveFn.create( cvs,
				    knots, deg,
				    MFnNurbsCurve::kOpen,
				    false, false,
				    MObject::kNullObj,
				    &stat );

    if ( MS::kSuccess != stat )
		cout<<"Error creating curve."<<endl;

}

MStatus motionTrace::redoIt()
//
// Description
//     This method performs the action of the command.
//
//     This method iterates over all selected items and
//     prints out connected plug and dependency node type
//     information.
//
{
	MStatus stat;				// Status code

	MObjectArray picked;
	MObject		dependNode;		// Selected dependency node

	// Create a selection list iterator
	//
	MSelectionList slist;
	MGlobal::getActiveSelectionList( slist );
	MItSelectionList iter( slist, MFn::kInvalid,&stat );

	// Iterate over all selected dependency nodes
	// and save them in a list
	//
	for ( ; !iter.isDone(); iter.next() )
	{
		// Get the selected dependency node
		//
		if ( MS::kSuccess != iter.getDependNode( dependNode ) )
		{
			cerr << "Error getting the dependency node" << endl;
			continue;
		}
		picked.append( dependNode );
	}

	// array of arrays for object position

	MPointArray *pointArrays = new MPointArray [ picked.length() ];

	unsigned int i;
	double time;

	//	Sample the animation using start, end, by values

	for ( time = start; time <= end; time+=by )
	{
		MTime timeval(time);

		MGlobal::viewFrame( timeval );

		// Iterate over selected dependency nodes
		//

		for ( i = 0; i < picked.length(); i++ )
		{
			// Get the selected dependency node
			//
			dependNode = picked[i];

			// Create a function set for the dependency node
			//
			MFnDependencyNode fnDependNode( dependNode );

			// Get the translation attribute values

			MObject txAttr;
			txAttr = fnDependNode.attribute( MString("translateX"), &stat );
			MPlug txPlug( dependNode, txAttr );
			double tx;
			stat = txPlug.getValue( tx );

			MObject tyAttr;
			tyAttr = fnDependNode.attribute( MString("translateY"), &stat );
			MPlug tyPlug( dependNode, tyAttr );
			double ty;
			stat = tyPlug.getValue( ty );

			MObject tzAttr;
			tzAttr = fnDependNode.attribute( MString("translateZ"), &stat );
			MPlug tzPlug( dependNode, tzAttr );
			double tz;
			stat = tzPlug.getValue( tz );

#if 0
			fprintf( stderr,
				     "Time = %2.2lf, XYZ = ( %2.2lf, %2.2lf, %2.2lf )\n\n",
					 time, tx, ty, tz );
#endif

			pointArrays[i].append( MPoint( tx, ty, tz )) ;
		}
	}

	// make a path curve for each selected object

	for ( i = 0; i < picked.length(); i++ )
		jMakeCurve( pointArrays[i] );

	delete [] pointArrays;
	return MS::kSuccess;
}


/////////////////////////////////////////////////////////////////
//
// The following routines are used to register/unregister
// the command we are creating within Maya
//
/////////////////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerCommand( "motionTrace", motionTrace::creator );
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status =  plugin.deregisterCommand( "motionTrace" );
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

	return status;
}
