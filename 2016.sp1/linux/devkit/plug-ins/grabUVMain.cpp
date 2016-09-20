// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.

// File: pluginMain.cpp
//
// Author: Maya Plug-in Wizard 2.0
//
#include <QtCore/QObject>
#include <QtGui/QApplication>
#include <QtGui/qevent.h>

#include <maya/MFnPlugin.h>
#include <maya/MIOStream.h>
#include <maya/MStatus.h>	  
#include <maya/MEvent.h> 
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MDagPath.h>
#include <maya/MFn.h>
#include <maya/MFnMesh.h>

#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>
#include <maya/MUintArray.h>
#include <maya/MToolsInfo.h>

#include <maya/MUIDrawManager.h>
#include <maya/MFrameContext.h>
#include <maya/MColor.h> 

#include <maya/MPxTexContext.h>
#include <maya/MPxContextCommand.h>
#include <maya/MPxPolyTweakUVInteractiveCommand.h>

class UVUpdateCommand : public MPxPolyTweakUVInteractiveCommand
{
public:
	static void *creator() { return new UVUpdateCommand; };
};

struct BrushConfig
{
	BrushConfig() : fSize(50.0f) {};
	float size() const { return fSize; }
	void setSize( float size ) { fSize = size; }

private:
	float fSize;
};

class grabUVContext : public QObject, public MPxTexContext
{
public:

	// constructor
	grabUVContext();

	// destructor
	virtual ~grabUVContext() {} ;

	virtual void	toolOnSetup( MEvent & event );
	virtual void	toolOffCleanup();

	virtual MStatus	doPress ( MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context);
	virtual MStatus	doRelease( MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context);
	virtual MStatus	doDrag ( MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context); 
	virtual MStatus	doPtrMoved ( MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context);
	virtual MStatus drawFeedback ( MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context);

	bool			doKeyPress( QKeyEvent* event );
	bool			doKeyRelease( QKeyEvent* event );

	virtual void	getClassName(MString & name) const;

	virtual bool	eventFilter(QObject *object, QEvent *event);

	float	size() const;
	void	setSize( float size );

private:		   	 

	enum DragMode
	{
		kNormal,
		kBrushSize
	};

	BrushConfig		fBrushConfig;
	DragMode		fDragMode;
	bool			fInStroke;

	MPoint			fCurrentPoint;
	MPoint			fLastPoint;
	MPoint			fBrushCenterScreenPoint;
	MPoint			fCurrentScreenPoint;
	MPoint			fLastScreenPoint;
	UVUpdateCommand *fCommand;
	MDagPath		fDagPath;
	MIntArray		fCollectedUVs;
	MGlobal::MSelectionMode	fPreviousSelectionMode;
	MSelectionMask	fPreviousSelectionMask;
}; 

grabUVContext::grabUVContext() :
	fCommand(NULL),
	fInStroke(false),
	fDragMode(kNormal)
{

}

float grabUVContext::size() const 
{ 
	return fBrushConfig.size();
}

void grabUVContext::setSize( float size )
{ 
	fBrushConfig.setSize( size ); 
	MToolsInfo::setDirtyFlag(*this); 
}

void grabUVContext::toolOnSetup( MEvent &event )
{
	MPxTexContext::toolOnSetup( event );

	QCoreApplication *app = qApp;
	app->installEventFilter(this);
}

void grabUVContext::toolOffCleanup()
{
	QCoreApplication *app = qApp;
	app->removeEventFilter(this);

	MPxTexContext::toolOffCleanup();
}

bool grabUVContext::eventFilter( QObject * object, QEvent *event )
{
	if( QKeyEvent* e = dynamic_cast<QKeyEvent*>(event) )
	{
		if( e->type() == QEvent::KeyPress )
			doKeyPress( e );
		else if( e->type() == QEvent::KeyRelease )
			doKeyRelease( e );
	}

	// if we return true, the event won't get propagated
	// to the rest of the widgets.
	return false;
}

bool grabUVContext::doKeyPress( QKeyEvent* event )
{
	if( fInStroke )
		return false;

	if( event->key() == Qt::Key_B ) 
	{
		fDragMode = kBrushSize;
		return true;
	}

	return false;
}

bool grabUVContext::doKeyRelease( QKeyEvent* event )
{
	if( fInStroke )
		return false;

	if( fDragMode != kNormal )
	{
		fDragMode = kNormal;
		return true;
	}

	return false;
}
MStatus	grabUVContext::doPress ( MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context)
{
	if ( event.mouseButton() != MEvent::kLeftMouse || 
		!event.isModifierNone() )
		return MS::kFailure;

	fInStroke = true;

	MPxTexContext::doPress(event, drawMgr, context);

	short x, y;
	event.getPosition( x, y );
	fCurrentScreenPoint = MPoint( x, y );
	fLastScreenPoint = MPoint( x, y );
	fBrushCenterScreenPoint = MPoint( x, y );

	double xView, yView;
	portToView(x, y, xView, yView);	// pos at viewrect coordinate  

	double portW, portH;
	portSize(portW, portH);

	double left, right, bottom, top;
	viewRect(left, right, bottom, top);

	double sizeInView = portW < 1e-5 ? 0.0 : ( fBrushConfig.size() * (right - left) / portW );
	double sizeInViewSquare = sizeInView * sizeInView;

	if( fDragMode == kNormal )
	{
		fCollectedUVs.clear();

		MStatus *returnStatus = NULL;
		MSelectionMask mask = MSelectionMask::kSelectMeshUVs;
		const bool bPickSingle = false;
		MSelectionList	selectionList;

		bool bSelect = MPxTexContext::getMarqueeSelection( x - fBrushConfig.size(), y - fBrushConfig.size(), 
														   x + fBrushConfig.size(), y + fBrushConfig.size(), 
														   mask, bPickSingle, true, selectionList );

		if (bSelect)
		{
			MObject component;
			selectionList.getDagPath( 0, fDagPath, component );
			fDagPath.extendToShape();

			MFnMesh mesh(fDagPath);

			MString currentUVSetName;
			mesh.getCurrentUVSetName(currentUVSetName);

			MIntArray UVsToTest;
			if( component.apiType() == MFn::kMeshMapComponent )
			{
				MFnSingleIndexedComponent compFn( component );
				compFn.getElements( UVsToTest );

				for (unsigned int i = 0; i < UVsToTest.length(); ++i)
				{
					float u, v;
					MStatus bGetUV = mesh.getUV(UVsToTest[i], u, v, &currentUVSetName);
					if (bGetUV == MS::kSuccess)
					{
						float distSquare = ( u - xView ) * ( u - xView ) + ( v - yView ) * ( v - yView );
						if ( distSquare < sizeInViewSquare )
							fCollectedUVs.append(UVsToTest[i]);
					}
				}

			}

			// position in view(world) space. 
			fLastPoint = MPoint( xView, yView, 0.0 );
			fCurrentPoint = MPoint( xView, yView, 0.0 );
		}
	}

	return MS::kSuccess;
}

MStatus	grabUVContext::doRelease( MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context)
{
	fInStroke = false;

	if (fCommand)
		fCommand->finalize();
	fCommand = NULL;

	MPxTexContext::doRelease(event, drawMgr, context);
	return MS::kSuccess;
}

MStatus	grabUVContext::doDrag ( MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context)
{
	if (event.mouseButton() != MEvent::kLeftMouse || 
		!event.isModifierNone() )
		return MS::kFailure;

	MPxTexContext::doDrag(event, drawMgr, context);

	short x, y;
	event.getPosition( x, y );
	fLastScreenPoint = fCurrentScreenPoint;
	fCurrentScreenPoint = MPoint( x, y );

	double xView, yView;
	portToView(x, y, xView, yView);	// pos at viewrect coordinate

	fLastPoint = fCurrentPoint;
	fCurrentPoint = MPoint( xView, yView, 0.0 );

	if( fDragMode == kBrushSize )
	{
		double dis = fCurrentScreenPoint.distanceTo( fLastScreenPoint );
		if ( fCurrentScreenPoint[0] > fLastScreenPoint[0] )
			setSize( size() + float(dis) );
		else
			setSize( std::max( size() - float(dis), 0.01f ) );
	}
	else
	{
		fBrushCenterScreenPoint = MPoint( x, y );

		MFloatArray uUVsExported;
		MFloatArray vUVsExported;

		const MVector vec = fCurrentPoint - fLastPoint;

		if (!fCommand)
		{
			fCommand = (UVUpdateCommand *)(newToolCommand());
		}
		if (fCommand)
		{
			MFnMesh mesh(fDagPath);
			MString currentUVSetName;
			mesh.getCurrentUVSetName(currentUVSetName);

			int nbUVs = mesh.numUVs(currentUVSetName);
			MDoubleArray pinData;
			MUintArray uvPinIds;
			MDoubleArray fullPinData;
			mesh.getPinUVs(uvPinIds, pinData, &currentUVSetName);
			int len = pinData.length();

			fullPinData.setLength(nbUVs);
			for (unsigned int i = 0; i < nbUVs; i++) {
				fullPinData[i] = 0.0;
			}
			while( len-- > 0 ) {
				fullPinData[uvPinIds[len]] = pinData[len];
			}

			MFloatArray uValues;
			MFloatArray vValues;
			float pinWeight = 0;
			for (unsigned int i = 0; i < fCollectedUVs.length(); ++i)
			{
				float u, v;
				MStatus bGetUV = mesh.getUV(fCollectedUVs[i], u, v, &currentUVSetName);
				if (bGetUV == MS::kSuccess)
				{
					pinWeight = fullPinData[fCollectedUVs[i]];
					u += (float)vec[0]*(1-pinWeight);
					v += (float)vec[1]*(1-pinWeight);
					uValues.append( u );
					vValues.append( v ); 
				}
			}
			fCommand->setUVs( mesh.object(), fCollectedUVs, uValues, vValues, &currentUVSetName );
		}
	}
	return MS::kSuccess;
}

MStatus grabUVContext::doPtrMoved( MEvent & event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context)
{
	MPxTexContext::doPtrMoved(event, drawMgr, context);

	double portW, portH;
	portSize(portW, portH);

	short x, y;
	event.getPosition( x, y );

	y = short(portH) - y;

	fCurrentScreenPoint = MPoint( x, y );
	fLastScreenPoint = MPoint( x, y );
	fBrushCenterScreenPoint = MPoint( x, y );

	return MS::kSuccess;
}

MStatus grabUVContext::drawFeedback(MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context)
{
	// to draw the brush ring.
	drawMgr.beginDrawable();
	{
		drawMgr.setColor( MColor(1.f, 1.f, 1.f) ); 
		drawMgr.setLineWidth( 2.0f );
		drawMgr.circle2d(MPoint(fBrushCenterScreenPoint.x, fBrushCenterScreenPoint.y), fBrushConfig.size());
	}				
	drawMgr.endDrawable();

	return MS::kSuccess;
}

void grabUVContext::getClassName( MString & name ) const
{
	name.set("grabUV");
}

#define kSizeFlag			"-sz"
#define kSizeFlagLong		"-size"

//---------------------------------------------
class grabUVContextCommand : public MPxContextCommand
{
public:	 
						grabUVContextCommand();
	virtual MStatus		doEditFlags();
	virtual MStatus		doQueryFlags();
	virtual MStatus		appendSyntax();
	virtual MPxContext* makeObj();
	static void*		creator();

protected:
	grabUVContext*		fGrabUVContext;
};		    

grabUVContextCommand::grabUVContextCommand() {}

MPxContext* grabUVContextCommand::makeObj()
//
// Description
//    When the context command is executed in maya, this method
//    be used to create a context.
//
{
	fGrabUVContext = new grabUVContext();
	return fGrabUVContext;
}

void* grabUVContextCommand::creator()
//
// Description
//    This method creates the context command.
//
{
	return new grabUVContextCommand;
}

MStatus grabUVContextCommand::doEditFlags()
{
	MStatus status = MS::kSuccess;

	MArgParser argData = parser();

	if (argData.isFlagSet(kSizeFlag)) {
		double size;
		status = argData.getFlagArgument(kSizeFlag, 0, size);
		if (!status) {
			status.perror("size flag parsing failed.");
			return status;
		}
		fGrabUVContext->setSize( float(size) );
	}

	return MS::kSuccess;
}

MStatus grabUVContextCommand::doQueryFlags()
{
	MArgParser argData = parser();

	if (argData.isFlagSet(kSizeFlag)) {
		setResult(fGrabUVContext->size());
	}

	return MS::kSuccess;
}

MStatus grabUVContextCommand::appendSyntax()
{
	MSyntax mySyntax = syntax();

	if (MS::kSuccess != mySyntax.addFlag(kSizeFlag, kSizeFlagLong,
		MSyntax::kDouble)) {
			return MS::kFailure;
	}

	return MS::kSuccess;
}

// ------------------------------------------------------------------
#define CTX_CREATOR_NAME "grabUVContext"
#define TEX_COMMAND_NAME "uvUpdateCommand" 

MStatus initializePlugin( MObject obj )
	//
	//	Description:
	//		this method is called when the plug-in is loaded into Maya.  It 
	//		registers all of the services that this plug-in provides with 
	//		Maya.
	//
	//	Arguments:
	//		obj - a handle to the plug-in object (use MFnPlugin to access it)
	//
{ 
	MStatus   status;
	MFnPlugin plugin( obj, "", "2015", "Any");

	// Add plug-in feature registration here
	//
	status = plugin.registerContextCommand(CTX_CREATOR_NAME, grabUVContextCommand::creator,
		TEX_COMMAND_NAME, UVUpdateCommand::creator);

	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}

MStatus uninitializePlugin( MObject obj )
	//
	//	Description:
	//		this method is called when the plug-in is unloaded from Maya. It 
	//		deregisters all of the services that it was providing.
	//
	//	Arguments:
	//		obj - a handle to the plug-in object (use MFnPlugin to access it)
	//
{
	MStatus   status;
	MFnPlugin plugin( obj );

	// Add plug-in feature deregistration here
	//
	status = plugin.deregisterContextCommand(CTX_CREATOR_NAME, TEX_COMMAND_NAME);

	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}


