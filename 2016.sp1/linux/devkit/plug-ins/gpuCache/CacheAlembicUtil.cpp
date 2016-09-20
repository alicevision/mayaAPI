//-
//**************************************************************************/
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.
//**************************************************************************/
//+

#include "CacheAlembicUtil.h"


namespace CacheAlembicUtil{
	// Global mutex for calls to Alembic library
	tbb::mutex gsAlembicMutex;

	const std::string kCustomPropertyWireIndices("adskWireIndices");
	const std::string kCustomPropertyWireIndicesOld("wireIndices");
	const std::string kCustomPropertyShadingGroupSizes("adskTriangleShadingGroupSizes");
    const std::string kCustomPropertyDiffuseColor("adskDiffuseColor");
	const std::string kCustomPropertyCreator("adskCreator");
	const std::string kCustomPropertyCreatorValue("adskGPUCache");
	const std::string kCustomPropertyVersion("adskVersion");
	const std::string kCustomPropertyVersionValue("1.0");

    const std::string kMaterialsObject("materials");
    const std::string kMaterialsGpuCacheTarget("adskMayaGpuCache");
    const std::string kMaterialsGpuCacheType("surface");
}








