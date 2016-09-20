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
#include <maya/MSimple.h>
#include <maya/MSelectionList.h>
#include <maya/M3dView.h>
#include <maya/MGlobal.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MDagPath.h>
#include <math.h>

DeclareSimpleCommand( listPolyHoles, PLUGIN_COMPANY, "4.5");

MStatus listPolyHoles::doIt( const MArgList& args )
//
//	Description:
//		Implements the MEL listPolyHoles command.  This command outputs a list
//		of all the holes in each selected polymesh.
//		
//	Arguments:
//		args - the argument list that was passes to the command from MEL.  This
//			   command takes no arguments.
//
//	Return Value:
//		MS::kSuccess - command succeeded
//		MS::kFailure - command failed (returning this value will cause the 
//                     MEL script that is being run to terminate unless the
//                     error is caught using a "catch" statement.
//
{
	MStatus stat = MS::kSuccess;

	MSelectionList curSel;
	MGlobal::getActiveSelectionList( curSel );

	cout<<endl;
	cout<<"*****************"<<endl;
	cout<<"* listPolyHoles *"<<endl;
	cout<<"*****************"<<endl;

	// iterate through the selection list, and find holes in any selected
	// polygons.
	//
	int numSelected = curSel.length();
	for( int s = 0; s < numSelected; s++ )
	{
		MDagPath dagPath;
		MObject node;

		// get the selected item, figure out if it's a polymesh or not
		//
		curSel.getDagPath( s, dagPath );
		if( dagPath.extendToShape() != MS::kSuccess )
		{
			// selection does not correspond to a DAG shape
			//
			cout<<"	Error - object is not a polymesh"<<endl;
			stat = MS::kFailure;
			return stat;
		}

		node = dagPath.node();
		MFnDependencyNode fnNode( node );
		cout<<endl<<"Looking for holes in "<<fnNode.name().asChar()<<endl;
		
		MStatus polyStatus;
		MFnMesh fnMesh( node, &polyStatus );

		if( polyStatus == MS::kSuccess )
		{
			// Retrieve the list of holes in the polymesh.
			// 'holeInfo' array stores 3 integers for each hole:
			// 
			//	[ face, numVertices, startIndex ]
			//
			// where 'face' is the index of the face in which the 
			// hole is found, 'numVertices' is the number of vertices
			// that define the hole, and 'startIndex' is the starting
			// index of the list of hole vertices in the 'holeVertices'
			// array.
			//
			MIntArray holeInfo;

			// 'holeVertices' array contains a linear listing of the
			// vertices comprising all the holes in the selected polymesh.
			// information from the 'holeInfo' array is used to index into
			// this list to extract the list of vertices for each hole.
			//
			MIntArray holeVertices;
			MStatus holeStatus;

			holeInfo.clear();
			holeVertices.clear();
			
			// get the holes
			//
			int numHoles = fnMesh.getHoles( holeInfo, holeVertices, &holeStatus );
			if( holeStatus == MS::kSuccess )
			{
				cout<<"    Poly has "<<numHoles<<" holes"<<endl;

				for( int h = 0; h < numHoles; h++ )
				{
					// retrieve the hole face index, number of hole vertices, and
					// hole vertex list start index for the current hole.
					//
					int holeFace = holeInfo[3*h];
					int holeNumVertices = holeInfo[3*h+1];
					int holeStartIndex = holeInfo[3*h+2];

					cout<<"    Hole "<<h<<":"<<endl;
					cout<<"        Face "<<holeFace<<endl;
					cout<<"        Start index "<<holeStartIndex<<endl;
					cout<<"        "<<holeNumVertices<<" vertices: ";

					// retrieve the hole vertex indices from the 'holeVertices'
					// array.  The indices are a list of 'holeNumVertices' entries,
					// starting at index 'holeStartIndex'.
					//
					for( int v = 0; v < holeNumVertices; v++ )
					{
						int index = holeStartIndex + v;
						cout<<holeVertices[index];
					}

					cout<<endl;
				}
				
			}
			else
			{
				// something went wrong trying to retrieve the poly holes
				//
				cout<<"    Error retrieving polygon holes"<<endl;
				stat = MS::kFailure;
			}
		}
		else
		{
			// the current selection item was not a polymesh, so we can't retrieve
			// holes for it.
			//
			cout<<"	Error - object is not a polymesh"<<endl;
			stat = MS::kFailure;
		}
	}
	
	setResult( "listPolyHoles completed." );
	return stat;
}




