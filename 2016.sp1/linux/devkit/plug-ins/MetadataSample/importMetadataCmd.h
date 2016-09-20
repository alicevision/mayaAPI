#ifndef _importMetadataCmd_h_
#define _importMetadataCmd_h_

#include "metadataBase.h"
#include <maya/MDGModifier.h>

//======================================================================
//
// Read in a data stream from a file
//
class importMetadataCmd : public metadataBase
{
public:
	static MSyntax	  cmdSyntax ();
	static void*		creator	();
	static const char*	   name	();

			importMetadataCmd	();
	virtual	~importMetadataCmd	();

	virtual bool	isUndoable	()			const;
	virtual MStatus	redoIt		();
	virtual MStatus	undoIt		();

protected:
	virtual MStatus	checkArgs	( MArgDatabase& argDb );
	virtual MStatus doCreate	();

private:
	//========================================
	//
	// Arguments specific to this command (excluding inherited args)
	//
	OptFlag<void,	 CommandMode(kCreate)> fMergeFlag;
	OptFlag<MString, CommandMode(kCreate)> fStringFlag;
	//
	//========================================
	
	bool	fMerge;		// If true then import without erasing existing data
	MString	fString;	// String to use instead of a file for importing

	MDGModifier	fDGModifier;	// Information needed for undo/redo
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
#endif // _importMetadataCmd_h_
