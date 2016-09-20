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

#include <maya/MPxLocatorNode.h> 
#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MMatrix.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MColor.h>
#include <maya/M3dView.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnNurbsSurfaceData.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MFnMatrixData.h>
#include <maya/MIOStream.h>

class cvColor : public MPxLocatorNode
{
public:
							cvColor();
	virtual					~cvColor(); 

    virtual MStatus   		compute( const MPlug& plug, MDataBlock& data );

	virtual void            draw( M3dView & view, const MDagPath & path, 
								  M3dView::DisplayStyle style,
								  M3dView::DisplayStatus status );

	virtual bool            isBounded() const;

	static  void *          creator();
	static  MStatus         initialize();

	static	MTypeId			id;
	static	MObject			drawingEnabled;
	static	MObject			pointSize;
	static  MObject         inputSurface;
	static	MObject			cvLocations;
};

MTypeId cvColor::id( 0x80006 );
MObject cvColor::inputSurface;
MObject cvColor::cvLocations;
MObject cvColor::pointSize;
MObject cvColor::drawingEnabled;

cvColor::cvColor() {}
cvColor::~cvColor() {}

MStatus cvColor::compute( const MPlug& plug, MDataBlock& data )
{ 
	MStatus stat;

	// cout << "cvColor::compute\n";

	if ( plug == cvLocations ) {
		MDataHandle inputData = data.inputValue ( inputSurface, &stat );
		if (!stat) {
			stat.perror("cvColor::compute get inputSurface");
			return stat;
		}

		MObject surf = inputData.asNurbsSurface();
		MFnNurbsSurface surfFn (surf, &stat);
		if (!stat) {
			stat.perror("cvColor::compute surface creator");
			return stat;
		}

		MDataHandle outputData = data.outputValue ( cvLocations, &stat );
		if (!stat) {
			stat.perror("cvColor::compute get cvLocations");
			return stat;
		}
		
		MObject cvs = outputData.data();
		MFnPointArrayData cvData(cvs, &stat);
		if (!stat) {
			stat.perror("cvColor::compute point array data creator");
			return stat;
		}

		MPointArray cvArray;
		stat = surfFn.getCVs( cvArray, MSpace::kObject);
		if (!stat) {
			stat.perror("cvColor::compute getCVs");
			return stat;
		}

		stat = cvData.set ( cvArray );
		if (!stat) {
			stat.perror("cvColor::compute setCVs");
			return stat;
		}

		outputData.set ( cvs );

		stat = data.setClean ( plug );
		if (!stat) {
			stat.perror("cvColor::compute setClean");
			return stat;
		}
	} else {
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}

void cvColor::draw( M3dView & view, const MDagPath & path, 
							 M3dView::DisplayStyle style,
							 M3dView::DisplayStatus status )
{ 
	// cout << "cvColor::draw\n";

	MStatus		stat;
	MObject		thisNode = thisMObject();

	MPlug enPlug( thisNode, drawingEnabled );
	bool doDrawing; 
	stat = enPlug.getValue ( doDrawing );
	if (!stat) {
		stat.perror("cvColor::draw get drawingEnabled");
		return;
	}

	if (!doDrawing)
		return;

	MPlug szPlug( thisNode, pointSize );
	float ptSize; 
	stat = szPlug.getValue ( ptSize );
	if (!stat) {
		stat.perror("cvColor::draw get pointSize");
		ptSize = 4.0;
	}

	MPlug cvPlug( thisNode, cvLocations );
	MObject cvObject;
	stat = cvPlug.getValue(cvObject);
	if (!stat) {
		stat.perror("cvColor::draw get cvObject");
		return;
	}

	MFnPointArrayData cvData(cvObject, &stat);
	if (!stat) {
		stat.perror("cvColor::draw get point array data");
		return;
	}

	MPointArray cvs = cvData.array( &stat );
	if (!stat) {
		stat.perror("cvColor::draw get point array");
		return;
	}

	// Extract the 'worldMatrix' attribute that is inherited from 'dagNode'
	//
	MFnDependencyNode fnThisNode( thisNode );
	MObject worldSpaceAttribute = fnThisNode.attribute( "worldMatrix" );
	MPlug matrixPlug( thisNode, worldSpaceAttribute);

	// 'worldMatrix' is an array so we must specify which element the plug
	// refers to
	matrixPlug = matrixPlug.elementByLogicalIndex (0);

	// Get the value of the 'worldMatrix' attribute
	//
	MObject matObject;
	stat = matrixPlug.getValue(matObject);
	if (!stat) {
		stat.perror("cvColor::draw get matObject");
		return;
	}

	MFnMatrixData matrixData(matObject, &stat);
	if (!stat) {
		stat.perror("cvColor::draw get world matrix data");
		return;
	}

	MMatrix worldSpace = matrixData.matrix( &stat );
	if (!stat) {
		stat.perror("cvColor::draw get world matrix");
		return;
	}

	view.beginGL(); 

	// Push the color settings
	// 
	glPushAttrib( GL_CURRENT_BIT | GL_POINT_BIT );
	glPointSize( ptSize );
	glDisable ( GL_POINT_SMOOTH );  // Draw square "points"

	glBegin( GL_POINTS );

		int numCVs = cvs.length();
		for (int i = 0; i < numCVs; i++) {
			// cout << "cv[" << i << "]: " << cvs[i] << ": ";
			MPoint		cv( cvs[i] );
			MColor		cvColor;

			cv *= worldSpace;

			if (cv.x < 0 && cv.y < 0) {
				// cout << "Red";
				cvColor.r = 1.0;
				cvColor.g = 0.0;
				cvColor.b = 0.0;
			} else if (cv.x < 0 && cv.y >= 0) {
				// cout << "Cyan";
				cvColor.r = 0.0;
				cvColor.g = 1.0;
				cvColor.b = 1.0;
			} else if (cv.x >= 0 && cv.y < 0) {
				// cout << "Blue";
				cvColor.r = 0.0;
				cvColor.g = 0.0;
				cvColor.b = 1.0;
			} else if (cv.x >= 0 && cv.y >= 0) {
				// cout << "Yellow";
				cvColor.r = 1.0;
				cvColor.g = 1.0;
				cvColor.b = 0.0;
			}
			// cout << endl;

			view.setDrawColor ( cvColor );
			glVertex3f( (float)cvs[i].x, (float)cvs[i].y, (float)cvs[i].z);
		} 
	glEnd();

	glPopAttrib();

	view.endGL();
}

bool cvColor::isBounded() const
{ 
	return false;
}

void* cvColor::creator()
{
	return new cvColor();
}

MStatus cvColor::initialize()
{ 
	MStatus				stat;
	MFnNumericAttribute	numericAttr;
	MFnTypedAttribute	typedAttr;

	drawingEnabled = numericAttr.create( "drawingEnabled", "en",
									MFnNumericData::kBoolean, 1, &stat );
	if (!stat) {
		stat.perror("create drawingEnabled attribute");
		return stat;
	}

	pointSize = numericAttr.create( "pointSize", "ps",
									MFnNumericData::kFloat, 4.0, &stat );
	if (!stat) {
		stat.perror("create pointSize attribute");
		return stat;
	}
	
	inputSurface = typedAttr.create( "inputSurface", "is",
							 MFnNurbsSurfaceData::kNurbsSurface, &stat);
	if (!stat) {
		stat.perror("create inputSurface attribute");
		return stat;
	}

	cvLocations = typedAttr.create( "cvLocations", "cv",
							 MFnPointArrayData::kPointArray, &stat);
	if (!stat) {
		stat.perror("create cvLocations attribute");
		return stat;
	}

	MPointArray			defaultPoints;
	MFnPointArrayData	defaultArray;
	MObject				defaultAttr;

	defaultPoints.clear(); // Empty array
	defaultAttr = defaultArray.create (defaultPoints);
	stat = typedAttr.setDefault(defaultAttr);
	if (!stat) {
		stat.perror("could not create default output attribute");
		return stat;
	}

	stat = addAttribute (drawingEnabled);
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute (pointSize);
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute (inputSurface);
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute (cvLocations);
		if (!stat) { stat.perror("addAttribute"); return stat;}

	stat = attributeAffects( inputSurface, cvLocations );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( drawingEnabled, cvLocations );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( pointSize, cvLocations );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
	
	return MS::kSuccess;
}

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerNode( "cvColor", cvColor::id, 
						 &cvColor::creator, &cvColor::initialize,
						 MPxNode::kLocatorNode );
	if (!status) {
		status.perror("registerNode");
		return status;
	}
	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterNode( cvColor::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	return status;
}
