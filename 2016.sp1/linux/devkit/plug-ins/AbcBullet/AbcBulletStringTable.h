//
// Description:
//  Simple macro based string definition file. Not all string elements
//  have been moved over to this file. However, the eventual goal is
//  to have them here so that they are a bit easier to localize if
//  necessary.
//

#ifndef __ABCBULLET_STRING_TABLE_H
#define __ABCBULLET_STRING_TABLE_H

#include <maya/MStringResource.h>
#include <maya/MStringResourceId.h>

// Define plugin id
#define kPluginId  "AbcBullet"

// ... Begin Error Strings

#define kDagPathOrderMissingErr 			MStringResourceId( kPluginId, "kDagPathOrderMissingErr", 			"The Bullet Initial State '^1s' is out of sync with the Rigid Set '^2s' to which it belongs. Please use the 'Reset Initial State' command to update the solve.")
#define kInvalidArgFile 			MStringResourceId( kPluginId, "kInvalidArgFile", 			"File incorrectly specified.")
#define kInvalidArgFrameRange 		MStringResourceId( kPluginId, "kInvalidArgFrameRange", 		"Frame Range incorrectly specified.")
#define kInvalidArgStep 			MStringResourceId( kPluginId, "kInvalidArgStep", 			"Step incorrectly specified.")
#define kInvalidArgFrameRelativeSample 			MStringResourceId( kPluginId, "kInvalidArgFrameRelativeSample", 			"Frame Relative Sample incorrectly specified.")
#define kInvalidArgPythonPerframeCallback 			MStringResourceId( kPluginId, "kInvalidArgPythonPerframeCallback", 			"pythonPerFrameCallback incorrectly specified.")
#define kInvalidArgMelPostJobCallback 			MStringResourceId( kPluginId, "kInvalidArgMelPostJobCallback", 			"melPostJobCallback incorrectly specified.")
#define kInvalidArgAttrPrefix 			MStringResourceId( kPluginId, "kInvalidArgAttrPrefix", 			"attrPrefix incorrectly specified.")
#define kInvalidArgPythonPostJobCallback 			MStringResourceId( kPluginId, "kInvalidArgPythonPostJobCallback", 			"pythonPostJobCallback incorrectly specified.")
#define kInvalidArgAttr 			MStringResourceId( kPluginId, "kInvalidArgAttr", 			"attr incorrectly specified.")
#define kInvalidArgUserAttrPrefix 			MStringResourceId( kPluginId, "kInvalidArgUserAttrPrefix", 			"userAttrPrefix incorrectly specified.")
#define kEmpty 			MStringResourceId( kPluginId, "kEmpty", 			"")
#define kEmpty 			MStringResourceId( kPluginId, "kEmpty", 			"")
#define kEmpty 			MStringResourceId( kPluginId, "kEmpty", 			"")

// ... End Error Strings

#endif // __ABCBULLET_STRING_TABLE_H
