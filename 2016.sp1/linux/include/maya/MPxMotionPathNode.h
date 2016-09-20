#ifndef _MPxMotionPathNode
#define _MPxMotionPathNode
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================

//
// CLASS:	MPxMotionPathNode
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES

#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MPxNode.h>
#include <maya/MEulerRotation.h>

// ****************************************************************************
// DECLARATIONS

class	MDataBlock;
class	MMatrix;

// ****************************************************************************
// CLASS DECLARATION (MPxMotionPathNode)

//! \ingroup OpenMaya MPx
//! \brief Base class for user defined motionPath nodes. 
/*!
   MPxMotionPathNode provides you with the ability to write your own motion
   path classes. 
*/
class OPENMAYAANIM_EXPORT MPxMotionPathNode : public MPxNode
{
public:
			MPxMotionPathNode();
	virtual	~MPxMotionPathNode();

	virtual MPxNode::Type type() const;

	//
	// Inherited attributes
	//

	//! u value
	static MObject uValue;

	//! front twist
	static MObject frontTwist;

	//! up twist
	static MObject upTwist;

	//! side twist
	static MObject sideTwist;

	//! flow node
	static MObject flowNode;

	//! path geometry
	static MObject pathGeometry;

	//! position marker time
	static MObject positionMarkerTime;

	//! orientation marker time
	static MObject orientationMarkerTime;

	//! follow
	static MObject follow;

	//! normal
	static MObject normal;

	//! inverse up
	static MObject inverseUp;

	//! inverse front
	static MObject inverseFront;

	//! front axis
	static MObject frontAxis;

	//! up axis
	static MObject upAxis;

	//! world up type
	static MObject worldUpType;

	//! world up vector
	static MObject worldUpVector;
	static MObject worldUpVectorX;
	static MObject worldUpVectorY;
	static MObject worldUpVectorZ;

	//! world up matrix
	static MObject worldUpMatrix;

	//! bank
	static MObject bank;

	//! bank scale
	static MObject bankScale;

	//! bank threshold
	static MObject bankThreshold;

	//! fraction mode
	static MObject fractionMode;

	//! update orientation markers
	static MObject updateOrientationMarkers;

	//! possible values for <b>worldUpType</b>
	enum worldUpVectorValue
	{
		//! Use the scene up vector as world up
		kUpScene,
		//! Use the object's up vector as world up
		kUpObject,
		//! Use the object's rotation up vector as world up
		kUpObjectRotation,
		//! Use the value of the <b>worldUpVector</b> plug as world up
		kUpVector,
		//! Use the path normal world up
		kUpNormal
    };

	//! The computed world space position
	static MObject allCoordinates;

	//! X-component of the computed world space position
	static MObject xCoordinate;

	//! Y-component of the computed world space position
	static MObject yCoordinate;

	//! Z-component of the computed world space position
	static MObject zCoordinate;

	//! The computed world space orientation matrix
	static MObject orientMatrix;

	//! The computed world space rotation
	static MObject rotate;

	//! Angle of rotation about the X axis
	static MObject rotateX;

	//! Angle of rotation about the Y axis
	static MObject rotateY;

	//! Angle of rotation about the Z axis
	static MObject rotateZ;

	//! The order of rotations for the <b>rotate</b> attribute
	static MObject rotateOrder;

	// A-la-carte access to the individual components of the motionPath
	// evaluator.
	//
	MPoint					position(
									MDataBlock&				data,
									double					f,
									MStatus*				status = NULL ) const;
	MStatus					getVectors(
									MDataBlock&				data,
									double					f,
									MVector&				front,
									MVector&				side,
									MVector&				up,
									const MVector*			worldUp = NULL ) const;
	MQuaternion				banking(
									MDataBlock&				data,
									double					f,
									const MVector&			worldUp,
									double					bankScale,
									double					bankLimit,
									MStatus*				status = NULL ) const;

	// Evaluator that follows the order the motionPath node calculates
	// the result. If you want to change how various aspects of the
	// calculation are performed, combine the individual methods defined
	// in the a-la-carte section above.
	//
	MStatus					evaluatePath(
									MDataBlock&				data,
									double					u,
									double					uRange,
									bool					wraparound,
									double					sideOffset,
									double					upOffset,
									bool					follow,
									bool					inverseFront,
									bool					inverseUp,
									int						frontAxis,
									int						upAxis,
									double					frontTwist,
									double					upTwist,
									double					sideTwist,
									bool					bank,
									double					bankScale,
									double					bankLimit,
									MPoint&					resultPosition,
									MMatrix&				resultOrientation ) const;

	// Utility methods.
	//
	double  				parametricToFractional(
									double					u,
									MStatus*				status = NULL ) const;
	double  				fractionalToParametric(
									double					f,
									MStatus*				status = NULL ) const;
	double					wraparoundFractionalValue(
									double					f,
									MStatus*				status = NULL ) const;
	MMatrix					matrix(
									const MVector&			front,
									const MVector&			side,
									const MVector&			up,
									int						frontAxisIdx = 1,
									int						upAxisIdx = 2,
									MStatus*				status = NULL ) const;

private:
	static void				initialSetup();
	static const char*		className();
	static void*			dataBlock( MDataBlock& );

};

#endif /* __cplusplus */
#endif /* _MPxMotionPathNode */
