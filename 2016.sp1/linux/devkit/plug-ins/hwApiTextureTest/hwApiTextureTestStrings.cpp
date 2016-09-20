#include "hwApiTextureTestStrings.h"

#include <maya/MStringResource.h>
#include <maya/MString.h>

namespace hwApiTextureTestStrings
{
	#define kPluginId  "hwApiTextureTest"

	// Common
	const MStringResourceId kErrorRenderer			( kPluginId, "kErrorRenderer", 			MString( "hwApiTextureTest : Failed to acquire renderer." ) );
	const MStringResourceId kErrorTargetManager		( kPluginId, "kErrorTargetManager", 	MString( "hwApiTextureTest : Failed to acquire target manager." ) );
	const MStringResourceId kErrorTextureManager	( kPluginId, "kErrorTextureManager",	MString( "hwApiTextureTest : Failed to acquire texture manager." ) );

	// Load specific
	const MStringResourceId kBeginLoadTest			( kPluginId, "kBeginLoadTest", 			MString( "hwApiTextureTest load start ..." ) );
	const MStringResourceId kEndLoadTest			( kPluginId, "kEndLoadTest", 			MString( "hwApiTextureTest load done." ) );
	const MStringResourceId kErrorLoadPathArg		( kPluginId, "kErrorLoadPathArg", 		MString( "hwApiTextureTest : Failed to parse path argument." ) );
	const MStringResourceId kErrorLoadNoTexture		( kPluginId, "kErrorLoadNoTexture", 	MString( "hwApiTextureTest : No texture found." ) );
	const MStringResourceId kErrorLoadTexture		( kPluginId, "kErrorLoadTexture",		MString( "Failed to load texture <<^1s>>." ) );
	const MStringResourceId kSuccessLoadTexture		( kPluginId, "kSuccessLoadTexture", 	MString( "Texture <<^1s>> loaded successfully." ) );
	const MStringResourceId kErrorTileTexture		( kPluginId, "kErrorTileTexture",		MString( "Failed to tile texture <<^1s>>." ) );
	const MStringResourceId kTileTransform			( kPluginId, "kTileTransform",			MString( "Texture UV scale ^1s,^2s, UV offset=^3s,^4s." ) );

	// Save specific
	const MStringResourceId kBeginSaveTest			( kPluginId, "kBeginSaveTest", 			MString( "hwApiTextureTest save start ..." ) );
	const MStringResourceId kEndSaveTest			( kPluginId, "kEndSaveTest", 			MString( "hwApiTextureTest save done." ) );
	const MStringResourceId kErrorSavePathArg		( kPluginId, "kErrorSavePathArg", 		MString( "hwApiTextureTest : Failed to parse path argument." ) );
	const MStringResourceId kErrorSaveFormatArg		( kPluginId, "kErrorSaveFormatArg", 	MString( "hwApiTextureTest : Failed to parse format argument." ) );
	const MStringResourceId kErrorSaveGrabArg		( kPluginId, "kErrorSaveGrabArg", 		MString( "hwApiTextureTest : Failed to grab screen pixels." ) );
	const MStringResourceId kErrorSaveAcquireTexture( kPluginId, "kErrorSaveAcquireTexture",MString( "hwApiTextureTest : Failed to acquire texture from screen pixels." ) );
	const MStringResourceId kErrorSaveTexture		( kPluginId, "kErrorSaveTexture",		MString( "Failed to save texture <<^1s>> <<^2s>>." ) );
	const MStringResourceId kSuccessSaveTexture		( kPluginId, "kSuccessSaveTexture",		MString( "Texture <<^1s>> <<^2s>> saved successfully." ) );

	// DX specific
	const MStringResourceId kDxErrorEffect			( kPluginId, "kDxErrorEffect", 			MString( "Failed to create effect <<^1s>>." ) );
	const MStringResourceId kDxErrorInputLayout		( kPluginId, "kDxErrorInputLayout", 	MString( "Failed to create input layout." ) );
}


// Register all strings
//
MStatus hwApiTextureTestStrings::registerMStringResources(void)
{
	// Common
	MStringResource::registerString( kErrorRenderer );
	MStringResource::registerString( kErrorTargetManager );
	MStringResource::registerString( kErrorTextureManager );

	// Load specific
	MStringResource::registerString( kBeginLoadTest );
	MStringResource::registerString( kEndLoadTest );
	MStringResource::registerString( kErrorLoadPathArg );
	MStringResource::registerString( kErrorLoadNoTexture );
	MStringResource::registerString( kErrorLoadTexture );
	MStringResource::registerString( kSuccessLoadTexture );

	// Save specific
	MStringResource::registerString( kBeginSaveTest );
	MStringResource::registerString( kEndSaveTest );
	MStringResource::registerString( kErrorSavePathArg );
	MStringResource::registerString( kErrorSaveFormatArg );
	MStringResource::registerString( kErrorSaveGrabArg );
	MStringResource::registerString( kErrorSaveAcquireTexture );
	MStringResource::registerString( kErrorSaveTexture );
	MStringResource::registerString( kSuccessSaveTexture );

	// DX specific
	MStringResource::registerString( kDxErrorEffect );
	MStringResource::registerString( kDxErrorInputLayout );

	return MS::kSuccess;
}

MString hwApiTextureTestStrings::getString(const MStringResourceId &stringId)
{
	MStatus status;
	return MStringResource::getString( stringId, status );
}

MString hwApiTextureTestStrings::getString(const MStringResourceId &stringId, const MString& arg)
{
	MString string;
	string.format( hwApiTextureTestStrings::getString( stringId ), arg );
	return string;
}

MString hwApiTextureTestStrings::getString(const MStringResourceId &stringId, const MString& arg1, const MString& arg2)
{
	MString string;
	string.format( hwApiTextureTestStrings::getString( stringId ), arg1, arg2 );
	return string;
}

MString hwApiTextureTestStrings::getString(const MStringResourceId &stringId, float arg1, float arg2, float arg3, float arg4)
{
	MString string;
	MString arg1String; arg1String += arg1;
	MString arg2String; arg2String += arg2;
	MString arg3String; arg3String += arg3;
	MString arg4String; arg4String += arg4;
	string.format( hwApiTextureTestStrings::getString( stringId ), arg1String, arg2String, arg3String, arg4String );
	return string;
}

//-
// Copyright 2013 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+
