#ifndef __AbcImportStrings_h__
#define __AbcImportStrings_h__
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.

#include <maya/MStringResourceId.h>

class MString;
class MStringArray;

namespace AbcImportStrings
{
	extern const MStringResourceId kErrorInvalidAlembic;
	extern const MStringResourceId kErrorConnectionNotFound;
	extern const MStringResourceId kErrorConnectionNotMade;
	extern const MStringResourceId kWarningNoAnimatedParticleSupport;
	extern const MStringResourceId kWarningUnsupportedAttr;
	extern const MStringResourceId kWarningSkipIndexNonArray;
	extern const MStringResourceId kWarningSkipOddlyNamed;
	extern const MStringResourceId kWarningSkipNoSamples;
    extern const MStringResourceId kAEAlembicAttributes;

		// Register all strings
	MStatus registerMStringResources(void);

	MString getString(const MStringResourceId &stringId);
}

#endif