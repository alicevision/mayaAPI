#ifndef __hwApiTextureTestStrings_h__
#define __hwApiTextureTestStrings_h__

#include <maya/MStringResourceId.h>

class MString;

namespace hwApiTextureTestStrings
{
	// Common
	extern const MStringResourceId kErrorRenderer;
	extern const MStringResourceId kErrorTargetManager;
	extern const MStringResourceId kErrorTextureManager;

	// Load specific
	extern const MStringResourceId kBeginLoadTest;
	extern const MStringResourceId kEndLoadTest;
	extern const MStringResourceId kErrorLoadPathArg;
	extern const MStringResourceId kErrorLoadNoTexture;
	extern const MStringResourceId kErrorLoadTexture;
	extern const MStringResourceId kSuccessLoadTexture;

	// Save specific
	extern const MStringResourceId kBeginSaveTest;
	extern const MStringResourceId kEndSaveTest;
	extern const MStringResourceId kErrorSavePathArg;
	extern const MStringResourceId kErrorSaveFormatArg;
	extern const MStringResourceId kErrorSaveGrabArg;
	extern const MStringResourceId kErrorSaveAcquireTexture;
	extern const MStringResourceId kErrorSaveTexture;
	extern const MStringResourceId kSuccessSaveTexture;

	// DX specific
	extern const MStringResourceId kDxErrorEffect;
	extern const MStringResourceId kDxErrorInputLayout;

	// Tiling specific
	extern const MStringResourceId kErrorTileTexture;
	extern const MStringResourceId kTileTransform;

	// Register all strings
	MStatus registerMStringResources(void);

	MString getString(const MStringResourceId &stringId);
	MString getString(const MStringResourceId &stringId, const MString& arg);
	MString getString(const MStringResourceId &stringId, const MString& arg1, const MString& arg2);
	MString getString(const MStringResourceId &stringId, float arg1, float arg2, float arg3, float arg4);
}

#endif

//-
// Copyright 2013 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+

