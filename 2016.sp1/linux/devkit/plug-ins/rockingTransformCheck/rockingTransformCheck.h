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
// Define declarations
//

#include "rockingTransform.h"

#define kRockingTransformCheckNodeID 0x8101D
#define kRockingTransformCheckMatrixID 0x8101E

#define ReturnOnError(status)			   \
		if (MS::kSuccess != status) {	   \
			return status;				  \
		}

//
// Class declarations -- matrix and transform
// NOTE: that we derive from the rockingTransform base
// classes so that we avoid code duplication.
//

class rockingTransformCheckMatrix : public rockingTransformMatrix
{
	// A really simple implementation of MPxTransformationMatrix.
	// The methods include:
	// - Two accessor methods for getting and setting the 
	// rock
	// - The virtual asMatrix() method which passes the matrix 
	// back to Maya when the command "xform -q -ws -m" is invoked

	public:
		rockingTransformCheckMatrix();
		static void *creator();
				
		static	MTypeId	idCheck;
	private:		
		typedef rockingTransformCheckMatrix ParentClass;
};

class rockingTransformCheckNode : public rockingTransformNode 
{
	public:
		// A really simple custom transform.
		rockingTransformCheckNode();
		rockingTransformCheckNode(MPxTransformationMatrix *);
		virtual ~rockingTransformCheckNode();

		virtual MPxTransformationMatrix *createTransformationMatrix();
										
		// Utility for getting the related rock matrix pointer
		rockingTransformCheckMatrix *getRockingTransformCheckMatrix();
				
		const char* className();
		static	void * 	creator();
		
		static	MTypeId	idCheck;
		
	protected:
		virtual	MEulerRotation applyRotationLocks(const MEulerRotation &toTest,
											const MEulerRotation &savedR,
											MStatus *ReturnStatus = NULL);
		virtual MEulerRotation applyRotationLimits(const MEulerRotation &unclampedR,
											  MDataBlock &block,
											  MStatus *ReturnStatus = NULL);
		virtual	MStatus checkAndSetRotation(MDataBlock &block,
											const MPlug&,
											const MEulerRotation&, 
											MSpace::Space = MSpace::kTransform);  

	
	private:
		typedef rockingTransformNode ParentClass;
};


