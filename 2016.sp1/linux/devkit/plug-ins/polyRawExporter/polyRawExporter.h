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

#ifndef __POLYRAWEXPORTER_H
#define __POLYRAWEXPORTER_H

// polyRawExporter.h

// *****************************************************************************
//
// CLASS:    polyRawExporter
//
// *****************************************************************************
//
// CLASS DESCRIPTION (polyRawExporter)
// 
// polyRawExporter is a class derived from polyExporter.  It allows the export
// of polygonal mesh data in raw text format.  The file extension for this type
// is ".raw".
//
// *****************************************************************************

#include "polyExporter.h"

class polyRawExporter : public polyExporter {

	public:
								polyRawExporter(){}
		virtual					~polyRawExporter();

		static	void*			creator();
				MString			defaultExtension () const;
				MStatus			initializePlugin(MObject obj);
				MStatus			uninitializePlugin(MObject obj);


	private:	
				polyWriter*		createPolyWriter(const MDagPath dagPath, MStatus& status);
				void			writeHeader(ostream& os);
};

#endif /*__POLYEXPORTER_H*/
