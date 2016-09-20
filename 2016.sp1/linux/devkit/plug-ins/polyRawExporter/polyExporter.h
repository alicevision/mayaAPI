#ifndef __POLYEXPORTER_H
#define __POLYEXPORTER_H

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

//	polyExporter.h

//
// *****************************************************************************
//
// CLASS:    polyExporter
//
// *****************************************************************************
//
// CLASS DESCRIPTION (polyExporter)
// 
// polyExporter is a class used for creating polygonal mesh exporter plugins.  
// It is derived from the MPxFileTranslator class and thus implements the
// functions writer(), haveWriteMethod(), haveReadMethod(), and canBeOpened(),
// which are called by Maya when a scene export is executed.  This class allows
// the choice of exporting all or only selected poly meshes in the scene.
//
// To use this class, derive a new class and begin by adding the following *.h 
// files:
// #include <maya/MFnPlugin.h> - used for defining plugins
// #include <maya/MIOStream.h> - used for input/output operations
// #include <maya/MFStream.h>  - used for file input/output operations
//
// The following functions must be implemented:
// creator() - required by Maya to allocate an instance of the derived class
// initializePlugin() - required to register the plugin with Maya
// uninitializePlugin() - required to unregister the plugin with Maya
// defaultExtension() - returns an the export file type extension
// createPolyWriter() - returns a new polyWriter which performs the exporting
//
// For examples, see the classes polyRawExporter and polyX3DExporter
//
// *****************************************************************************

#include <maya/MPxFileTranslator.h>

class polyWriter;
class MDagPath;
class MFnDagNode;

class polyExporter:public MPxFileTranslator {

	public:
								polyExporter();
		virtual					~polyExporter();

		virtual MStatus			writer (const MFileObject& file,
										const MString& optionsString,
										MPxFileTranslator::FileAccessMode mode);
		virtual bool			haveWriteMethod () const;
		virtual bool			haveReadMethod () const;
		virtual	bool			canBeOpened () const;

		virtual MString			defaultExtension () const = 0;


	protected:	
		virtual	bool			isVisible(MFnDagNode& fnDag, MStatus& status);
		virtual	MStatus			exportAll(ostream& os);
		virtual	MStatus			exportSelection(ostream& os);
		virtual void			writeHeader(ostream& os);
		virtual void			writeFooter(ostream& os);
		virtual MStatus			processPolyMesh(const MDagPath dagPath, ostream& os);
		virtual polyWriter*		createPolyWriter(const MDagPath dagPath, MStatus& status) = 0;
};

#endif /*__POLYEXPORTER_H*/
