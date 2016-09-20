#ifndef _metadataBase_h_
#define _metadataBase_h_

#include <string>
#include "cmdUtils.h"
#include <maya/MFn.h>
#include <maya/MPxCommand.h>
#include <maya/MObjectArray.h>

// Forward declarations
namespace adsk { namespace Data { class AssociationsSerializer; } }
class MFileObject;

///////////////////////////////////////////////////////////////////////////////
//
// Base class for data stream commands. Extracts common functionality such
// as identifying the objects and stream component types.
//
////////////////////////////////////////////////////////////////////////////////


class metadataBase : public MPxCommand
{
public:
	static MSyntax cmdSyntax	();

	metadataBase				();
	virtual ~metadataBase		();

	// No creator because this is a top-level class, not an actual command

	virtual MStatus	doIt		( const MArgList& );
	virtual MStatus	redoIt		();
	virtual MStatus	undoIt		();

	virtual bool	isUndoable	()						const;
	virtual bool 	hasSyntax	()						const;

protected:
	virtual MStatus checkArgs	( MArgDatabase& argDb );
	virtual MStatus doCreate	();
	virtual MStatus doEdit		();
	virtual MStatus doQuery		();

	//========================================
	//
	// Arguments common to all derived commands
	//
	OptFlag<MString, CommandMode(kCreate)> fFileFlag;
	OptFlag<MString, CommandMode(kCreate)> fMetadataFormatFlag;
	//
	//========================================

protected:
	CommandMode		fMode;	 	// Command mode
	MObjectArray	fObjects;	// Object(s) to which the command applies
	MFileObject*	fFile;		// File to use for operations
	const adsk::Data::AssociationsSerializer* fSerializer; // Serialization format
};

//-
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//+

#endif // _metadataBase_h_
