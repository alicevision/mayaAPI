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

#ifndef __POLYX3DWRITER_H
#define __POLYX3DWRITER_H

// polyX3DWriter.h

//
// *****************************************************************************
//
// CLASS:    polyX3DWriter
//
// *****************************************************************************
//
// CLASS DESCRIPTION (polyX3DWriter)
// 
// polyX3DWriter is a class derived from polyWriter.  It currently outputs, in
// X3D compliant format, the following polygonal mesh data:
// - faces and their vertex components
// - vertex coordinates
// - colors per vertex
// - normals per vertex
// - current uv set and coordinates
// - component sets
// - file textures (for the current uv set)
//
// *****************************************************************************

#include "polyWriter.h"

class polyX3DWriter:public polyWriter {

	public:
						polyX3DWriter (const MDagPath& dagPath, MStatus& status);
		virtual			~polyX3DWriter ();
				MStatus extractGeometry ();
				MStatus writeToFile (ostream& os);

	private:
		//Methods
		//
				MStatus	outputSingleSet (ostream& os, 
										 MString setName, 
										 MIntArray faces, 
										 MString textureName);
				MStatus outputX3DShapeTag (ostream& os,
										   const MString shapeName,
										   const MIntArray& faces,
										   const MString textureName,
										   const unsigned int tabCount);
				MStatus outputX3DAppearanceTag (ostream& os, 
												const MString textureName, 
												const unsigned int tabCount);
				MStatus outputX3DIndexedFaceSetTag (ostream& os, 
													const MIntArray& faces, 
													const MString textureName, 
													unsigned int tabCount);
				MStatus outputX3DCoordinateTag (ostream& os, const unsigned int tabCount);
				MStatus outputX3DTextureCoordinateTag (ostream& os, const unsigned int tabCount);
				MStatus outputX3DNormalTag (ostream& os, const unsigned int tabCount);
				MStatus outputX3DColorTag (ostream& os, const unsigned int tabCount);

		//Data Members
		//for keeping track of already outputted X3D tags
		//
		short int			fTagFlags;

		//for formatting purposes - this changes depending on whether or not
		//a group node is being outputted
		//
		short int			fInitialTabCount;

		//for storing UV information
		//
		MFloatArray			fUArray;
		MFloatArray			fVArray;
};

#endif /*__POLYX3DWRITER_H*/
