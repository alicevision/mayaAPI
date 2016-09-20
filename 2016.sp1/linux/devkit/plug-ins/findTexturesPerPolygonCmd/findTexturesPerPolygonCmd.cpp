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
// Registers a new command called "findTexturesPerPolygon".
// Takes a selected mesh and outputs polygonal sets with file textures applied
// to a "color" attribute, and members of each set. The output is to stderr.
//

#include <maya/MIOStream.h>
#include <maya/MSimple.h>
#include <maya/MString.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSet.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MObjectArray.h>
#include <maya/MPlug.h>

MObject findShader( MObject& setNode )
//
//  Description:
//      Find the shading node for the given shading group set node.
//
{
	MFnDependencyNode fnNode(setNode);
	MPlug shaderPlug = fnNode.findPlug("surfaceShader");
			
	if (!shaderPlug.isNull()) {			
		MPlugArray connectedPlugs;
		bool asSrc = false;
		bool asDst = true;
		shaderPlug.connectedTo( connectedPlugs, asDst, asSrc );

		if (connectedPlugs.length() != 1)
			cerr << "Error getting shader\n";
		else 
			return connectedPlugs[0].node();
	}			
	
	return MObject::kNullObj;
}

DeclareSimpleCommand( findTexturesPerPolygon, PLUGIN_COMPANY, "3.0" );

MStatus findTexturesPerPolygon::doIt( const MArgList& )
//
//  Description:
//      Find the texture files that apply to the color of each polygon of
//      a selected shape if the shape has its polygons organized into sets.
//
{
    // Get the selection and choose the first path on the selection list.
    //
	MStatus status;
	MDagPath path;
	MObject cmp;
	MSelectionList slist;
	MGlobal::getActiveSelectionList(slist);
	slist.getDagPath(0, path, cmp);

	// Have to make the path include the shape below it so that
	// we can determine if the underlying shape node is instanced.
	// By default, dag paths only include transform nodes.
	//
	path.extendToShape();

	// If the shape is instanced then we need to determine which
	// instance this path refers to.
	//
	int instanceNum = 0;
	if (path.isInstanced())
		instanceNum = path.instanceNumber();

    // Get a list of all sets pertaining to the selected shape and the
    // members of those sets.
    //
	MFnMesh fnMesh(path);
	MObjectArray sets;
	MObjectArray comps;
	if (!fnMesh.getConnectedSetsAndMembers(instanceNum, sets, comps, true))
		cerr << "ERROR: MFnMesh::getConnectedSetsAndMembers\n";

	// Loop through all the sets.  If the set is a polygonal set, find the
    // shader attached to the and print out the texture file name for the
    // set along with the polygons in the set.
	//
	for ( unsigned i=0; i<sets.length(); i++ ) {
		MObject set = sets[i];
		MObject comp = comps[i];

		MFnSet fnSet( set, &status );
		if (status == MS::kFailure) {
            cerr << "ERROR: MFnSet::MFnSet\n";
            continue;
        }

        // Make sure the set is a polygonal set.  If not, continue.
		MItMeshPolygon piter(path, comp, &status);
		if ((status == MS::kFailure) || comp.isNull())
            continue;

		// Find the texture that is applied to this set.  First, get the
		// shading node connected to the set.  Then, if there is an input
		// attribute called "color", search upstream from it for a texture
		// file node.
        //
		MObject shaderNode = findShader(set);
		if (shaderNode == MObject::kNullObj)
			continue;

		MPlug colorPlug = MFnDependencyNode(shaderNode).findPlug("color", &status);
		if (status == MS::kFailure)
			continue;

		MItDependencyGraph dgIt(colorPlug, MFn::kFileTexture,
						   MItDependencyGraph::kUpstream, 
						   MItDependencyGraph::kBreadthFirst,
						   MItDependencyGraph::kNodeLevel, 
						   &status);

		if (status == MS::kFailure)
			continue;
		
		dgIt.disablePruningOnFilter();

        // If no texture file node was found, just continue.
        //
		if (dgIt.isDone())
			continue;
		  
        // Print out the texture node name and texture file that it references.
        //
		MObject textureNode = dgIt.thisNode();
        MPlug filenamePlug = MFnDependencyNode(textureNode).findPlug("fileTextureName");
        MString textureName;
        filenamePlug.getValue(textureName);
		cerr << "Set: " << fnSet.name() << endl;
        cerr << "Texture Node Name: " << MFnDependencyNode(textureNode).name() << endl;
		cerr << "Texture File Name: " << textureName.asChar() << endl;
        
        // Print out the set of polygons that are contained in the current set.
        //
		for ( ; !piter.isDone(); piter.next() )
			cerr << "    poly component: " << piter.index() << endl;
	}

	return MS::kSuccess;
}
