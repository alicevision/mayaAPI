// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.

#include "AbcImportStrings.h"

#include <maya/MStringResource.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>

namespace AbcImportStrings
{
	#define kPluginId  "AbcImport"

	const MStringResourceId kErrorInvalidAlembic					( kPluginId, "kErrorInvalidAlembic", 				MString( "is not a valid Alembic file" ) );
	const MStringResourceId kErrorConnectionNotFound				( kPluginId, "kErrorConnectionNotFound", 			MString( "not found for connection" ) );
	const MStringResourceId kErrorConnectionNotMade					( kPluginId, "kErrorConnectionNotMade", 			MString( "connection not made" ) );
	const MStringResourceId kWarningNoAnimatedParticleSupport		( kPluginId, "kWarningNoAnimatedParticleSupport", 	MString( "Currently no support for animated particle system" ) );
	const MStringResourceId kWarningUnsupportedAttr					( kPluginId, "kWarningUnsupportedAttr", 	MString( "Unsupported attr, skipping: " ) );
	const MStringResourceId kWarningSkipIndexNonArray				( kPluginId, "kWarningSkipIndexNonArray", 	MString( "Skipping indexed or non-array property: " ) );
	const MStringResourceId kWarningSkipOddlyNamed					( kPluginId, "kWarningSkipOddlyNamed", 	MString( "Skipping oddly named property: " ) );
	const MStringResourceId kWarningSkipNoSamples					( kPluginId, "kWarningSkipNoSamples", 	MString( "Skipping property with no samples: " ) );
	const MStringResourceId kAEAlembicAttributes					( kPluginId, "kAEAlembicAttributes", 	MString( "Alembic Attributes" ) );
}

//String registration
MStatus AbcImportStrings::registerMStringResources(void)
{
	MStringResource::registerString( kErrorInvalidAlembic );
	MStringResource::registerString( kErrorConnectionNotFound );
	MStringResource::registerString( kErrorConnectionNotMade );
	MStringResource::registerString( kWarningNoAnimatedParticleSupport );	
	MStringResource::registerString( kWarningUnsupportedAttr );
	MStringResource::registerString( kWarningSkipIndexNonArray );
	MStringResource::registerString( kWarningSkipOddlyNamed );
	MStringResource::registerString( kWarningSkipNoSamples );
    MStringResource::registerString( kAEAlembicAttributes  );

	return MS::kSuccess;
}

//string retrieval
MString AbcImportStrings::getString(const MStringResourceId &stringId)
{
	MStatus status;
	return MStringResource::getString(stringId, status);
}
