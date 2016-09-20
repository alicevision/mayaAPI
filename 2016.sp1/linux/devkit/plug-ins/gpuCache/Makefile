#-
# ==========================================================================
# Copyright (c) 2011 Autodesk, Inc.
# All rights reserved.
# 
# These coded instructions, statements, and computer programs contain
# unpublished proprietary information written by Autodesk, Inc., and are
# protected by Federal copyright law. They may not be disclosed to third
# parties or copied or duplicated in any form, in whole or in part, without
# the prior written consent of Autodesk, Inc.
# ==========================================================================
#+

ifndef INCL_BUILDRULES

#
# Include platform specific build settings
#
TOP := ..
include $(TOP)/buildrules


#
# Always build the local plug-in when make is invoked from the
# directory.
#
all : prebuiltPlugins

endif

#
# Variable definitions
#

SRCDIR := $(TOP)/gpuCache
DSTDIR := $(TOP)/gpuCache

gpuCache_SOURCES  := $(TOP)/gpuCache/gpuCacheGLFT.cpp $(TOP)/gpuCache/gpuCacheConfig.cpp $(TOP)/gpuCache/gpuCacheVramQuery.cpp $(TOP)/gpuCache/gpuCacheVBOProxy.cpp $(TOP)/gpuCache/gpuCacheSample.cpp $(TOP)/gpuCache/gpuCacheGeometry.cpp $(TOP)/gpuCache/gpuCacheFrustum.cpp $(TOP)/gpuCache/gpuCacheDrawTraversal.cpp $(TOP)/gpuCache/gpuCacheMaterial.cpp $(TOP)/gpuCache/gpuCacheMaterialNodes.cpp $(TOP)/gpuCache/gpuCacheMaterialBakers.cpp $(TOP)/gpuCache/gpuCacheUnitBoundingBox.cpp $(TOP)/gpuCache/gpuCacheSelect.cpp $(TOP)/gpuCache/gpuCacheRasterSelect.cpp $(TOP)/gpuCache/gpuCacheGLPickingSelect.cpp $(TOP)/gpuCache/gpuCacheShapeNode.cpp $(TOP)/gpuCache/gpuCacheDrawOverride.cpp $(TOP)/gpuCache/gpuCacheSubSceneOverride.cpp $(TOP)/gpuCache/gpuCacheSpatialSubdivision.cpp $(TOP)/gpuCache/gpuCacheSpatialGrid.cpp $(TOP)/gpuCache/gpuCacheSpatialGridWalker.cpp $(TOP)/gpuCache/gpuCacheIsectUtil.cpp $(TOP)/gpuCache/gpuCacheUtil.cpp $(TOP)/gpuCache/gpuCacheCmd.cpp $(TOP)/gpuCache/CacheAlembicUtil.cpp $(TOP)/gpuCache/CacheWriter.cpp $(TOP)/gpuCache/CacheWriterAlembic.cpp $(TOP)/gpuCache/CacheReader.cpp $(TOP)/gpuCache/CacheReaderAlembic.cpp $(TOP)/gpuCache/gpuCachePluginMain.cpp
gpuCache_OBJECTS  := $(TOP)/gpuCache/gpuCacheGLFT.o $(TOP)/gpuCache/gpuCacheConfig.o $(TOP)/gpuCache/gpuCacheVramQuery.o $(TOP)/gpuCache/gpuCacheVBOProxy.o $(TOP)/gpuCache/gpuCacheSample.o $(TOP)/gpuCache/gpuCacheGeometry.o $(TOP)/gpuCache/gpuCacheFrustum.o $(TOP)/gpuCache/gpuCacheDrawTraversal.o $(TOP)/gpuCache/gpuCacheMaterial.o $(TOP)/gpuCache/gpuCacheMaterialNodes.o $(TOP)/gpuCache/gpuCacheMaterialBakers.o $(TOP)/gpuCache/gpuCacheUnitBoundingBox.o $(TOP)/gpuCache/gpuCacheSelect.o $(TOP)/gpuCache/gpuCacheRasterSelect.o $(TOP)/gpuCache/gpuCacheGLPickingSelect.o $(TOP)/gpuCache/gpuCacheShapeNode.o $(TOP)/gpuCache/gpuCacheDrawOverride.o $(TOP)/gpuCache/gpuCacheSubSceneOverride.o $(TOP)/gpuCache/gpuCacheSpatialSubdivision.o $(TOP)/gpuCache/gpuCacheSpatialGrid.o $(TOP)/gpuCache/gpuCacheSpatialGridWalker.o $(TOP)/gpuCache/gpuCacheIsectUtil.o $(TOP)/gpuCache/gpuCacheUtil.o $(TOP)/gpuCache/gpuCacheCmd.o $(TOP)/gpuCache/CacheAlembicUtil.o $(TOP)/gpuCache/CacheWriter.o $(TOP)/gpuCache/CacheWriterAlembic.o $(TOP)/gpuCache/CacheReader.o $(TOP)/gpuCache/CacheReaderAlembic.o $(TOP)/gpuCache/gpuCachePluginMain.o
gpuCache_PLUGIN   := $(DSTDIR)/gpuCache.$(EXT)
gpuCache_MAKEFILE := $(DSTDIR)/Makefile

#
# Include the optional per-plugin Makefile.inc
#
#    The file can contain macro definitions such as:
#       {pluginName}_EXTRA_CFLAGS
#       {pluginName}_EXTRA_C++FLAGS
#       {pluginName}_EXTRA_INCLUDES
#       {pluginName}_EXTRA_LIBS
-include $(SRCDIR)/Makefile.inc


#
# Set target specific flags.
#

$(gpuCache_OBJECTS): CFLAGS   := $(CFLAGS)   $(gpuCache_EXTRA_CFLAGS)
$(gpuCache_OBJECTS): C++FLAGS := $(C++FLAGS) $(gpuCache_EXTRA_C++FLAGS)
$(gpuCache_OBJECTS): INCLUDES := $(INCLUDES) $(gpuCache_EXTRA_INCLUDES)

depend_gpuCache:     INCLUDES := $(INCLUDES) $(gpuCache_EXTRA_INCLUDES)

$(gpuCache_PLUGIN):  LFLAGS   := $(LFLAGS) $(gpuCache_EXTRA_LFLAGS) 
$(gpuCache_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaAnim -lOpenMayaRender -lFoundation -lGL -lAlembicAbcCoreFactory -lAlembicAbcCoreHDF5 -lAlembicAbcCoreOgawa -lAlembicAbcGeom -lAlembicAbc -lAlembicAbcCoreAbstract -lAlembicAbcMaterial -lAlembicAbcCollection -lAlembicOgawa -lAlembicUtil -lhdf5_hl -lhdf5 -lAlembicIex -lAlembicHalf -lAlembicImath -lz -ltbb $(gpuCache_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_gpuCache clean_gpuCache Clean_gpuCache


$(gpuCache_PLUGIN): $(gpuCache_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_gpuCache :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(gpuCache_SOURCES)

clean_gpuCache:
	-rm -f $(gpuCache_OBJECTS)

Clean_gpuCache:
	-rm -f $(gpuCache_MAKEFILE).bak $(gpuCache_OBJECTS) $(gpuCache_PLUGIN)


prebuiltPlugins: $(gpuCache_PLUGIN)
depend:	 depend_gpuCache
clean:	 clean_gpuCache
Clean:	 Clean_gpuCache

