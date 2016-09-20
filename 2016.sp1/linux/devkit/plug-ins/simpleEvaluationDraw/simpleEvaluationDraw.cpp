//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

// Plugin: simpleEvaluationDraw.cpp

// This plug-in demonstrates the use of MPxNode::postEvaluation() method. If
// Maya is running in Serial or Parallel Evaluation Manager mode then it is possible
// to use the postEvaluation() method to perform heavy calculations for 
// rendering.  In the example below, we use a time attribute
// and a copy attribute to perform a calculation that will slow
// Maya down.  Switching from Serial to Parallel Evaluation Mode will 
// show an increase in frame rate as the postEvaluation method will be called from 
// a separate thread as Maya starts to do more processing at the same time.
// In DG evaluation mode, the slow calculation will be done in the drawing code.
//

/*
    // Run the following script to make 20 locator nodes
    //
    loadPlugin simpleEvaluationDraw;

    for ( $i = 0 ; $i < 20; $i++ )
    {
        string $n = `createNode simpleEvaluationDraw`;
        string $dest = ( $n + ".inputTime" );
        connectAttr time1.outTime $dest;
    }

    // 0. Turn on frame rate display
    // 1. Start playback
    // 2. Turn on DG Evaluation Mode
    // 2. Switch to Serial Evaluation Mode 
    // 3. Switch to Parallel Evaluation Mode and check the frame rate

*/

#include <stdlib.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MPxLocatorNode.h>
#include <maya/MEvaluationNode.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MTime.h>

// Viewport 2.0 includes
#include <maya/MDrawRegistry.h>
#include <maya/MPxDrawOverride.h>
#include <maya/MUserData.h>

using namespace MHWRender;

class simpleEvaluationDraw : public MPxLocatorNode
{
public:
	simpleEvaluationDraw();
	virtual ~simpleEvaluationDraw();

	static void*			creator();
	static MStatus			initialize();

    virtual MStatus			postEvaluation( const  MDGContext& context, const MEvaluationNode& evaluationNode, PostEvaluationType evalType ); 

    // time input
    static  MObject     aTimeInput;
    // copies
    static MObject      aCopies;

    double scaleXBy;    // Scale value
    bool scaleUpToDate; // Is scale up to date

public:
    // Calculation for slowing down Maya and showing how postEvaluation works
    double doExpensiveCalculation( int c, double t );

	static	MTypeId		id;
	static	MString		drawDbClassification;
	static	MString		drawRegistrantId;
};

MObject simpleEvaluationDraw::aTimeInput;
MObject simpleEvaluationDraw::aCopies;

MTypeId simpleEvaluationDraw::id(0x0008002C);
MString	simpleEvaluationDraw::drawDbClassification("drawdb/geometry/simpleEvaluationDraw");
MString	simpleEvaluationDraw::drawRegistrantId("simpleEvaluationDrawPlug");

simpleEvaluationDraw::simpleEvaluationDraw()
    : scaleXBy( 1.0 )
    , scaleUpToDate( false )
{}

simpleEvaluationDraw::~simpleEvaluationDraw() {}

void* simpleEvaluationDraw::creator()
{
	return new simpleEvaluationDraw();
}

// Calculation to simulate an expensive operation which would
// slow Maya down
//
double simpleEvaluationDraw::doExpensiveCalculation( int c, double t )
{
    // Depending on your machine speed, you may have to tweak the
    // calculation or add a sleep to get Maya to slow down
    //
    unsigned int end = c * c *c * c * c;
    double result = 0;
    for ( unsigned int i = 0; i < end ; i++ )
        result = result + i*c*t;
    result = fmod(result,7.0)+1.0;
    return result;
}

MStatus simpleEvaluationDraw::postEvaluation( const  MDGContext& context, const MEvaluationNode& evaluationNode, PostEvaluationType evalType )
{
    if( !context.isNormal() ) 
        return MStatus::kFailure;

    MStatus status;
    if(evalType == kLeaveDirty)
    {
        scaleUpToDate = false;
    }
    else if ( (evaluationNode.dirtyPlugExists(aCopies, &status) && status ) || 
              ( evaluationNode.dirtyPlugExists(aTimeInput, &status) && status ) )
    {
        MDataBlock block = forceCache();
        MDataHandle inputTimeData = block.inputValue( aTimeInput, &status );
        if ( status )
        {
            MDataHandle copiesData = block.inputValue( aCopies, &status );
            if ( status )
            {
                // A made up calculation to slow down processing
                //
                MTime time = inputTimeData.asTime();
                int copies = copiesData.asInt();
                double t = time.value();
                if ( ! scaleUpToDate )
                {
                    scaleXBy = doExpensiveCalculation( copies, t );
                    // Mark the scale as up to date so that draw does not
                    // have to recompute it
                    scaleUpToDate = true;
                }
            }
        }
    }
   return MStatus::kSuccess;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Viewport 2.0 override implementation
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class simpleEvaluationDrawData : public MUserData 
{
public:
	simpleEvaluationDrawData();
	virtual ~simpleEvaluationDrawData() {};

	MColor rectangleColor;
    double scaleXBy;
    bool scaleUpToDate;

    double evalTime;
    double copies;
};

simpleEvaluationDrawData::simpleEvaluationDrawData() : MUserData(false) 
	, rectangleColor(1.0f, 0.0f, 0.0f, 1.0f)	
    , scaleXBy( 0.0 )
    , scaleUpToDate( false )
{
}

class simpleEvaluationDrawOverride : public MPxDrawOverride
{
public:
	static MPxDrawOverride* Creator(const MObject& obj)
	{
		return new simpleEvaluationDrawOverride(obj);
	}

	virtual ~simpleEvaluationDrawOverride();

	virtual DrawAPI supportedDrawAPIs() const;

	virtual bool isBounded(
		const MDagPath& objPath,
		const MDagPath& cameraPath) const;

	virtual MBoundingBox boundingBox(
		const MDagPath& objPath,
		const MDagPath& cameraPath) const;

	virtual MUserData* prepareForDraw(
		const MDagPath& objPath,
		const MDagPath& cameraPath,
		const MFrameContext& frameContext,
		MUserData* oldData);

	virtual bool hasUIDrawables() const { return true; }

	virtual void addUIDrawables(
		const MDagPath& objPath,
		MHWRender::MUIDrawManager& drawManager,
		const MHWRender::MFrameContext& frameContext,
		const MUserData* data);

	static void draw(const MHWRender::MDrawContext& context, const MUserData* data);

protected:

private:
	simpleEvaluationDrawOverride(const MObject& obj);
};

simpleEvaluationDrawOverride::simpleEvaluationDrawOverride(const MObject& obj)
	: MPxDrawOverride(obj, simpleEvaluationDrawOverride::draw)
{
}

simpleEvaluationDrawOverride::~simpleEvaluationDrawOverride()
{
}

DrawAPI simpleEvaluationDrawOverride::supportedDrawAPIs() const
{
	// this plugin supports both GL and DX
	return (kOpenGL | kDirectX11 | kOpenGLCoreProfile);
}

bool simpleEvaluationDrawOverride::isBounded(const MDagPath& /*objPath*/,
	const MDagPath& /*cameraPath*/) const
{
	return false;
}

MBoundingBox simpleEvaluationDrawOverride::boundingBox(
	const MDagPath& objPath,
	const MDagPath& cameraPath) const
{
	return MBoundingBox();
}

MUserData* simpleEvaluationDrawOverride::prepareForDraw(
	const MDagPath& objPath,
	const MDagPath& cameraPath,
	const MHWRender::MFrameContext& frameContext,
	MUserData* oldData)
{
	simpleEvaluationDrawData* data = dynamic_cast<simpleEvaluationDrawData*>(oldData);
	if (!data) {
		data = new simpleEvaluationDrawData();
	}

	MStatus status;
	MObject drawNode = objPath.node(&status);
	if (status) 
    {
		// retrieve color
		{
            // Normally this value would be taken from a plug
            float blue = (float) rand()/RAND_MAX;
            data->rectangleColor.b = blue;
		}
        // get the scaleXBy value
        {
            MFnDependencyNode dnNode(drawNode, &status);
            if ( status )
            {
                simpleEvaluationDraw* sed = dynamic_cast<simpleEvaluationDraw*>(dnNode.userNode());
                if ( sed )
                {
                    if ( ! sed->scaleUpToDate )
                    {
                        // Scale is not up to date so we must calculate the value
                        MPlug timePlug( drawNode, simpleEvaluationDraw::aTimeInput );
                        MPlug copiesPlug( drawNode, simpleEvaluationDraw::aCopies );
                        MTime t = timePlug.asMTime();
                        sed->scaleXBy = sed->doExpensiveCalculation( copiesPlug.asInt(), t.value() );
                    }
                    data->scaleXBy = sed->scaleXBy;
                    sed->scaleUpToDate = true;
                }
            }
        }
	}

	return data;
}

void simpleEvaluationDrawOverride::addUIDrawables(
	const MDagPath& objPath,
	MHWRender::MUIDrawManager& drawManager,
	const MHWRender::MFrameContext& frameContext,
	const MUserData* data)
{
	const simpleEvaluationDrawData* thisdata = dynamic_cast<const simpleEvaluationDrawData*>(data);
	if (!thisdata) {
		return;
	}

    drawManager.beginDrawable();
    {
        drawManager.setColor(thisdata->rectangleColor);
        drawManager.setLineWidth(2.0f);
        drawManager.setLineStyle(MUIDrawManager::kSolid);

        double xpos = rand()/RAND_MAX*10.0;
        MPoint position(xpos, 0.0, 0.5 );
        MVector normal(0.0, 0.0, 1.0);
        MVector rectUp(0.0, 1.0, 0.0);
        drawManager.rect(position, rectUp, normal, 5 * thisdata->scaleXBy, 5, false );
    }
    drawManager.endDrawable();

    MStatus status;
    MObject drawNode = objPath.node(&status);
    if ( status )
        {
        MFnDependencyNode dnNode(drawNode, &status);
        if ( status )
        {
            simpleEvaluationDraw* sed = dynamic_cast<simpleEvaluationDraw*>(dnNode.userNode());
            if ( sed )
                sed->scaleUpToDate = false; // Reset for next draw change
        }
    }
}

void simpleEvaluationDrawOverride::draw(const MHWRender::MDrawContext& context, const MUserData* data)
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Plug-in Registration
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

MStatus simpleEvaluationDraw::initialize()
{
	MStatus status;

	MFnNumericAttribute nAttr;
	MFnUnitAttribute    uAttr;

	aTimeInput = uAttr.create( "inputTime", "itm", MFnUnitAttribute::kTime, 0.0 );
	uAttr.setWritable(true);
	uAttr.setStorable(true);
    uAttr.setReadable(true);
    uAttr.setKeyable(true);
	MPxNode::addAttribute(aTimeInput);

    aCopies = nAttr.create("copies", "cp", MFnNumericData::kInt, 10 );
    nAttr.setMin(1);
    nAttr.setMax(50);
    MPxNode::addAttribute(aCopies);

	return MS::kSuccess;
}

MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerNode(
		"simpleEvaluationDraw",
		simpleEvaluationDraw::id,
		&simpleEvaluationDraw::creator,
		&simpleEvaluationDraw::initialize,
		MPxNode::kLocatorNode,
		&simpleEvaluationDraw::drawDbClassification);
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	status = MHWRender::MDrawRegistry::registerDrawOverrideCreator(
		simpleEvaluationDraw::drawDbClassification,
		simpleEvaluationDraw::drawRegistrantId,
		simpleEvaluationDrawOverride::Creator);
	if (!status) {
		status.perror("registerDrawOverrideCreator");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = MDrawRegistry::deregisterGeometryOverrideCreator(
		simpleEvaluationDraw::drawDbClassification,
		simpleEvaluationDraw::drawRegistrantId);
	if (!status) {
		status.perror("deregisterGeometryOverrideCreator");
		return status;
	}

	status = plugin.deregisterNode( simpleEvaluationDraw::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	return status;
}
