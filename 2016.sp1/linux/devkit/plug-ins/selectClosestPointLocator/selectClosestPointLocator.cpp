//Include files

// C++ library
// ----------------
#include <iostream>
#include <assert.h>
#include <math.h>
#include <limits>

#include "selectClosestPointLocator.h"

// Maya library
// ---------------
#include <maya/MMatrix.h>
#include <maya/MFnNumericAttribute.h>

#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h>

#define SCPL_EPSILON std::numeric_limits<double>::epsilon()

/////////////////////////
//
//
MTypeId selectClosestPointLocator::d_id(0x00081050);

MObject selectClosestPointLocator::aPlaneSizeAttr;
MObject selectClosestPointLocator::aNumDivsAttr;

selectClosestPointLocator::selectClosestPointLocator()
{
}

/***
*
*  Destructor
*/
selectClosestPointLocator::~selectClosestPointLocator()
{
}

/***
*
*  Creator
*/
void* selectClosestPointLocator::creator()
{
    return (void*) new selectClosestPointLocator;
}



/***
*
*  Initialize DG node
*/
MStatus
selectClosestPointLocator::initialize()
{
    MStatus status;

    MFnNumericAttribute nAttr;

    aPlaneSizeAttr =
        nAttr.create("planeSize", "psz", MFnNumericData::kDouble, 5.0, &status);

    nAttr.setStorable(true);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    nAttr.setKeyable(true);
    nAttr.setHidden(false);
	nAttr.setMin(.1);

    status = addAttribute(aPlaneSizeAttr);

    aNumDivsAttr =
        nAttr.create("numDivisions", "nd", MFnNumericData::kInt, 5, &status);
    nAttr.setStorable(true);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    nAttr.setKeyable(true);
    nAttr.setHidden(false);
    nAttr.setMin(1);

    status = addAttribute(aNumDivsAttr);

    return MS::kSuccess;
}

void selectClosestPointLocator::draw( M3dView & view,
                 const MDagPath & path,
                 M3dView::DisplayStyle style, M3dView::DisplayStatus displayStatus)
{
	
    MStatus status;
    MDataBlock  data=forceCache();
    MDataHandle handle = data.inputValue(aPlaneSizeAttr, &status);
    double planeSize = handle.asDouble();

    handle = data.inputValue(aNumDivsAttr, &status);
    int numDivs = handle.asInt();

    view.beginGL();

    glPushAttrib( GL_ALL_ATTRIB_BITS);

	int i, j;
	float min = (float)-planeSize, max = (float)planeSize;
	float step = (float)(( planeSize * 2.0 ) / (float)numDivs);

	//LINES
	view.setDrawColor(MColor(0.0,0.0,0.0));

	float u = min;
	glBegin(GL_LINES);
	for(i=0; i<numDivs+1; i++ )
	{
		glVertex3f(u,0.0,min);
		glVertex3f(u,0.0,max);
		u+=step;
	}
	glEnd();

	float v = min;
	glBegin(GL_LINES);
	for(j=0; j<numDivs+1; j++ )
	{
		glVertex3f(min,0.0,v);
		glVertex3f(max,0.0,v);
		v+=step;
	}
	glEnd();

	//FACES
    bool isHighlighted = displayStatus == M3dView::kActive ||
        displayStatus == M3dView::kLead;
	if(isHighlighted)
		view.setDrawColor(MColor(0.0, 1.0, 0.0));
	else{
		view.setDrawColor(MColor(1.0, 0.0, 1.0));
	}
	u = min;
	for(i=0; i<numDivs; i++ )
	{
		glBegin(GL_TRIANGLE_STRIP);
		v=min;
		for(j=0; j<numDivs+1; j++)
		{
			//top
			glVertex3f(u,0.0,v);
			//bottom
			glVertex3f(u+step,0.0,v);
			v+= step;
		}
		glEnd();
		u+= step;
	}

    glPopAttrib();

    view.endGL();
}

MPoint selectClosestPointLocator::closestPoint(MPoint cursorRayPoint, MVector cursorRayDir) const
{
	MString str("PLUGIN - inside selectClosestPointLocator::closestPoint\n");
	printf("\nPLUGIN - inside selectClosestPointLocator::closestPoint\n");

	// We can assume that this ray does actually intersect the shape, since it has already 
	// passed the hit/miss test.
	
	// Since this plugin is just a simple plane, we can easily compute the intersection 
	// of the cursor ray, which is in local space.
	//
	MPoint point;

	//plane normal (0,1,0)
	//
	//cursorRayDir.y * t + cursorRayPoint.y = 0
	double t;

	// NOTE: In case of the very rare case of the camera being co-planar with the locator plane, 
	// this would divide by zero.  For this simple example plugin, we are just returning the 
	// locator's origin in that case (by not modifying the initial value of the MPoint).
	//
	if ( fabs(cursorRayDir.y) < SCPL_EPSILON )
	{
		point.x = 0.0;
		point.y = 0.0;
		point.z = 0.0;
		point.w = 1.0;
	}else{
		t = (-cursorRayPoint.y) / cursorRayDir.y;
		point = (cursorRayDir * t) + cursorRayPoint;
	}

	printf("Ray Point: %f %f %f\n",cursorRayPoint.x, cursorRayPoint.y, cursorRayPoint.z);
	str += "Ray Point: ";
	str += cursorRayPoint.x;
	str += ", ";
	str += cursorRayPoint.y;
	str += ", ";
	str += cursorRayPoint.z;

	printf("Ray Dir:   %f %f %f\n",cursorRayDir.x, cursorRayDir.y, cursorRayDir.z);
	str += "\nRay Dir:   ";
	str += cursorRayDir.x;
	str += ", ";
	str += cursorRayDir.y;
	str += ", ";
	str += cursorRayDir.z;
	str += "\n";

	printf("Intersection Point: %f %f %f\n",point.x, point.y, point.z);
	str += "\nIntersection Point: ";
	str += point.x;
	str += ", ";
	str += point.y;
	str += ", ";
	str += point.z;
	str += "\n";
	MGlobal::displayInfo(str);

	return point;
}
        

MStatus initializePlugin(MObject obj)
{
    MStatus status;
    
    MFnPlugin plugin(obj, "Selection Target Node", "1.0", "any");
    
    status = plugin.registerNode("selectClosestPointLocator", 
                                 selectClosestPointLocator::d_id, 
                                 selectClosestPointLocator::creator, 
                                 selectClosestPointLocator::initialize,
                                 MPxNode::kLocatorNode);

    return status;
}

 MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    
    MFnPlugin plugin(obj);
                                 
    status = plugin.deregisterNode(selectClosestPointLocator::d_id);
    
    return status;
}


