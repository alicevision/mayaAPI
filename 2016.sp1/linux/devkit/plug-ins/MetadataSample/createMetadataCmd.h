#ifndef _createMetadataCmd_h_
#define _createMetadataCmd_h_

#include "cmdUtils.h"
#include <maya/MFn.h>
#include <maya/MPxCommand.h>
#include <maya/MObjectArray.h>
#include <maya/MDGModifier.h>
#include <maya/MIntArray.h>
#include <maya/adskDataAssociations.h>
namespace adsk { namespace Data { class Structure; } }

//======================================================================
//
// Create a set of randomized metadata on a channel stream
//
class createMetadataCmd : public MPxCommand
{
public:
	static MSyntax	  cmdSyntax ();
	static void*		creator	();
	static const char*	   name	();

			createMetadataCmd	();
	virtual	~createMetadataCmd	();

	virtual bool	isUndoable	()			const;
	virtual MStatus	doIt		( const MArgList& );
	virtual MStatus	redoIt		();
	virtual MStatus	undoIt		();

protected:
	virtual MStatus	checkArgs	( MArgDatabase& argDb );

private:
	//========================================
	//
	// Arguments specific to this command (excluding inherited args)
	//
	OptFlag<MString, CommandMode(kCreate)>	fChannelNameFlag;
	OptFlag<MString, CommandMode(kCreate)>	fStreamNameFlag;
	OptFlag<MString, CommandMode(kCreate)>	fStructureFlag;
	//
	//========================================
	
	std::string		fChannelName; // Channel on which to create metadata
	adsk::Data::Structure*
					fStructure;	  // Structure to use for creation
	MObjectArray	fNodes;		  // Node(s) on which to create metadata
	MDGModifier		fDGModifier;  // Information needed for undo/redo
	MIntArray		fIndexList;	  // Sorted list of indices to be retrieved
	MString			fStreamName;  // Name to give the new stream
};

//-
// ==================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
// 
// This computer source code  and related  instructions and comments are
// the unpublished confidential and proprietary information of Autodesk,
// Inc. and are  protected  under applicable  copyright and trade secret
// law. They may not  be disclosed to, copied or used by any third party
// without the prior written consent of Autodesk, Inc.
// ==================================================================
//+
#endif // _createMetadataCmd_h_
