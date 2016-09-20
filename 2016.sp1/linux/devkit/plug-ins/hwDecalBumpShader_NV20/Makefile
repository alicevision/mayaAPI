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
all : thirdPartyPlugins

endif

#
# Variable definitions
#

SRCDIR := $(TOP)/hwDecalBumpShader_NV20
DSTDIR := $(TOP)/hwDecalBumpShader_NV20

hwDecalBumpShader_NV20_SOURCES  := $(TOP)/hwDecalBumpShader_NV20/hwDecalBumpShader_NV20.cpp $(TOP)/hwDecalBumpShader_NV20/MNormalMapConverter.cpp $(TOP)/hwDecalBumpShader_NV20/MTexture.cpp $(TOP)/hwDecalBumpShader_NV20/MTextureCache.cpp $(TOP)/hwDecalBumpShader_NV20/NodeMonitor.cpp $(TOP)/hwDecalBumpShader_NV20/ShadingConnection.cpp
hwDecalBumpShader_NV20_OBJECTS  := $(TOP)/hwDecalBumpShader_NV20/hwDecalBumpShader_NV20.o $(TOP)/hwDecalBumpShader_NV20/MNormalMapConverter.o $(TOP)/hwDecalBumpShader_NV20/MTexture.o $(TOP)/hwDecalBumpShader_NV20/MTextureCache.o $(TOP)/hwDecalBumpShader_NV20/NodeMonitor.o $(TOP)/hwDecalBumpShader_NV20/ShadingConnection.o
hwDecalBumpShader_NV20_PLUGIN   := $(DSTDIR)/hwDecalBumpShader_NV20.$(EXT)
hwDecalBumpShader_NV20_MAKEFILE := $(DSTDIR)/Makefile

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

$(hwDecalBumpShader_NV20_OBJECTS): CFLAGS   := $(CFLAGS)   $(hwDecalBumpShader_NV20_EXTRA_CFLAGS)
$(hwDecalBumpShader_NV20_OBJECTS): C++FLAGS := $(C++FLAGS) $(hwDecalBumpShader_NV20_EXTRA_C++FLAGS)
$(hwDecalBumpShader_NV20_OBJECTS): INCLUDES := $(INCLUDES) $(hwDecalBumpShader_NV20_EXTRA_INCLUDES)

depend_hwDecalBumpShader_NV20:     INCLUDES := $(INCLUDES) $(hwDecalBumpShader_NV20_EXTRA_INCLUDES)

$(hwDecalBumpShader_NV20_PLUGIN):  LFLAGS   := $(LFLAGS) $(hwDecalBumpShader_NV20_EXTRA_LFLAGS) 
$(hwDecalBumpShader_NV20_PLUGIN):  LIBS     := $(LIBS)   -lFoundation -lOpenMaya -lOpenMayaUI -lGL -lGLU $(hwDecalBumpShader_NV20_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_hwDecalBumpShader_NV20 clean_hwDecalBumpShader_NV20 Clean_hwDecalBumpShader_NV20


$(hwDecalBumpShader_NV20_PLUGIN): $(hwDecalBumpShader_NV20_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_hwDecalBumpShader_NV20 :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(hwDecalBumpShader_NV20_SOURCES)

clean_hwDecalBumpShader_NV20:
	-rm -f $(hwDecalBumpShader_NV20_OBJECTS)

Clean_hwDecalBumpShader_NV20:
	-rm -f $(hwDecalBumpShader_NV20_MAKEFILE).bak $(hwDecalBumpShader_NV20_OBJECTS) $(hwDecalBumpShader_NV20_PLUGIN)


thirdPartyPlugins: $(hwDecalBumpShader_NV20_PLUGIN)
depend:	 depend_hwDecalBumpShader_NV20
clean:	 clean_hwDecalBumpShader_NV20
Clean:	 Clean_hwDecalBumpShader_NV20

