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
//	File Name:	animExportUtil.h
//
//	Description: an animation export utility which illustrates how to
//	use the MAnimUtil animation helper class, as well as how to export
//	animation using the Maya .anim format
//
//

#ifndef _animExportUtil
#define _animExportUtil

// *****************************************************************************

// INCLUDED HEADER FILES

#include <maya/MPxFileTranslator.h>
#include <maya/MFStream.h>

// *****************************************************************************

// DECLARATIONS

class MDagPath;
class MObject;

// *****************************************************************************

// CLASS DECLARATION (TanimExportUtil)

// The TanimExportUtil command object
//
class TanimExportUtil : public MPxFileTranslator {
public:
	TanimExportUtil ();
	virtual ~TanimExportUtil (); 

	virtual MStatus 	writer (
							const MFileObject &file,
							const MString &optionsString,
							FileAccessMode mode
						);
protected:
	void				write (ofstream &animFile, const MDagPath &path);
	void				write (ofstream &animFile, const MObject &node);
	void				writeAnimatedPlugs (
							ofstream &animFile,
							const MPlugArray &animatedPlugs,
							const MString &nodeName,
							unsigned int depth,
							unsigned int childCount
						);
public:
	virtual bool		haveWriteMethod () const;
	virtual MString 	defaultExtension () const;
	virtual MFileKind	identifyFile (
							const MFileObject &,
							const char *buffer,
							short size
						) const;

	static void *		creator ();
};

#endif
