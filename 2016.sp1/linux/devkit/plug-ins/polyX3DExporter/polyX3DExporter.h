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

#ifndef __POLYX3DEXPORTER_H
#define __POLYX3DEXPORTER_H

// polyX3DExporter.h

//
// *****************************************************************************
//
// CLASS:    polyX3DExporter
//
// *****************************************************************************
//
// CLASS DESCRIPTION (polyX3DExporter)
// 
// polyX3DExporter is a class derived from polyExporter.  It allows the export
// of polygonal mesh data in X3D compliant format.  The file extension for this 
// type is ".x3d".
//
// *****************************************************************************

#include "polyExporter.h"

class polyWriter;
class MObject;

class polyX3DExporter:public polyExporter {

	public:
								polyX3DExporter(){}
		virtual					~polyX3DExporter();

		static	void*			creator();
				MString			defaultExtension () const;
				MStatus			initializePlugin(MObject obj);
				MStatus			uninitializePlugin(MObject obj);


	private:	
				polyWriter*		createPolyWriter(const MDagPath dagPath, MStatus& status);
				void			writeHeader(ostream& os);
				void			writeFooter(ostream& os);
};

#endif /*__POLYX3DEXPORTER_H*/
