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

#ifndef _cleanPerFaceAssignmentCmd
#define _cleanPerFaceAssignmentCmd


#include <maya/MPxCommand.h>

class MArgList;

class cleanPerFaceAssignment : public MPxCommand
{

public:
				cleanPerFaceAssignment();
	virtual		~cleanPerFaceAssignment();

	MStatus		doIt( const MArgList& );
	static		void* creator();

};

#endif
